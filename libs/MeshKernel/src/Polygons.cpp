//---- GPL ---------------------------------------------------------------------
//
// Copyright (C)  Stichting Deltares, 2011-2021.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 3.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// contact: delft3d.support@deltares.nl
// Stichting Deltares
// P.O. Box 177
// 2600 MH Delft, The Netherlands
//
// All indications and logos of, and references to, "Delft3D" and "Deltares"
// are registered trademarks of Stichting Deltares, and remain the property of
// Stichting Deltares. All rights reserved.
//
//------------------------------------------------------------------------------

#include <MeshKernel/Constants.hpp>
#include <MeshKernel/Exceptions.hpp>
#include <MeshKernel/Operations.hpp>
#include <MeshKernel/Polygons.hpp>
#include <MeshKernel/TriangulationWrapper.hpp>

using meshkernel::Polygons;

Polygons::Polygons(const std::vector<Point>& polygon, Projection projection) : m_nodes(polygon), m_projection(projection)
{
    // Find the polygons in the current list of points
    m_outer_polygons_indices = FindIndices(polygon, 0, polygon.size(), constants::missing::doubleValue);
    for (size_t i = 0; i < m_outer_polygons_indices.size(); ++i)
    {
        m_inner_polygons_indices[i] = std::vector<std::pair<size_t, size_t>>{};

        const auto& [outer_start, outer_end] = m_outer_polygons_indices[i];

        // The inner polygon indices, the first interval corresponds to the outer polygon
        const auto inner_polygons_indices = FindIndices(polygon, outer_start, outer_end, constants::missing::innerOuterSeparator);

        // No inner polygon found
        if (inner_polygons_indices.size() <= 1)
        {
            continue;
        }

        // The first inner
        const auto inner_start = inner_polygons_indices[1].first;

        // store inner polygons for this outer polygon
        auto inner_polygons = std::vector<std::pair<size_t, size_t>>{};
        for (size_t j = 1; j < inner_polygons_indices.size(); ++j)
        {
            inner_polygons.emplace_back(inner_polygons_indices[j]);
        }

        m_inner_polygons_indices[i] = inner_polygons;

        // shift the index of the outer polygon, the
        m_outer_polygons_indices[i].second = inner_start - 2;
    }
}

std::vector<std::vector<meshkernel::Point>> Polygons::ComputePointsInPolygons() const
{

    std::vector generatedPoints(GetNumPolygons(), std::vector<Point>());
    std::vector<Point> localPolygon(GetNumNodes());
    TriangulationWrapper triangulationWrapper;

    for (size_t polygonIndex = 0; polygonIndex < m_outer_polygons_indices.size(); ++polygonIndex)
    {
        const auto& [outerStart, outerEnd] = m_outer_polygons_indices[polygonIndex];

        localPolygon.clear();
        for (auto j = outerStart; j <= outerEnd; ++j)
        {
            localPolygon.emplace_back(m_nodes[j]);
        }

        // not a closed polygon
        const auto numLocalPoints = localPolygon.size();
        if (localPolygon[numLocalPoints - 1] != localPolygon[0] || localPolygon.size() < 4)
        {
            continue;
        }

        const auto [localPolygonArea, centerOfMass, isCounterClockWise] = FaceAreaAndCenterOfMass(localPolygon, m_projection);

        const auto perimeter = PerimeterClosedPolygon(localPolygon);

        // average triangle size
        const auto averageEdgeLength = perimeter / static_cast<double>(numLocalPoints);
        const double averageTriangleArea = 0.25 * constants::numeric::squareRootOfThree * averageEdgeLength * averageEdgeLength;

        // estimated number of triangles
        constexpr size_t SafetySize = 11;
        const auto numberOfTriangles = static_cast<size_t>(SafetySize * localPolygonArea / averageTriangleArea);
        if (numberOfTriangles == 0)
        {
            throw AlgorithmError("Polygons::ComputePointsInPolygons: The number of triangles is <= 0.");
        }

        triangulationWrapper.Compute(localPolygon,
                                     TriangulationWrapper::TriangulationOptions::GeneratePoints,
                                     averageTriangleArea,
                                     numberOfTriangles);

        generatedPoints[polygonIndex].reserve(triangulationWrapper.GetNumNodes());
        for (auto i = 0; i < triangulationWrapper.GetNumNodes(); ++i)
        {
            if (Point p(triangulationWrapper.GetXCoord(i), triangulationWrapper.GetYCoord(i)); IsPointInPolygon(p, polygonIndex))
            {
                generatedPoints[polygonIndex].emplace_back(p);
            }
        }
    }

    return generatedPoints;
}

std::vector<meshkernel::Point> Polygons::RefineFirstPolygon(size_t startIndex,
                                                            size_t endIndex,
                                                            double refinementDistance) const
{
    if (m_outer_polygons_indices.empty())
    {
        throw std::invalid_argument("Polygons::RefineFirstPolygon: No nodes in polygon.");
    }

    if (startIndex == 0 && endIndex == 0)
    {
        const auto& [outerStart, outerEnd] = m_outer_polygons_indices[0];
        startIndex = outerStart;
        endIndex = outerEnd;
    }

    if (endIndex <= startIndex)
    {
        throw std::invalid_argument("Polygons::RefineFirstPolygon: The end index is smaller than the start index.");
    }

    bool areIndicesValid = false;
    size_t polygonIndex;
    for (size_t i = 0; i < GetNumPolygons(); ++i)
    {
        const auto& [outerStart, outerEnd] = m_outer_polygons_indices[i];
        if (startIndex >= outerStart && endIndex <= outerEnd)
        {
            areIndicesValid = true;
            polygonIndex = i;
            break;
        }
    }

    if (!areIndicesValid)
    {
        throw std::invalid_argument("Polygons::RefineFirstPolygon: The indices are not valid.");
    }

    const auto edgeLengths = PolygonEdgeLengths(m_nodes);
    std::vector<double> nodeLengthCoordinate(edgeLengths.size());
    nodeLengthCoordinate[0] = 0.0;
    for (size_t i = 1; i < edgeLengths.size(); ++i)
    {
        nodeLengthCoordinate[i] = nodeLengthCoordinate[i - 1] + edgeLengths[i - 1];
    }

    const auto numNodesRefinedPart = size_t(std::ceil((nodeLengthCoordinate[endIndex] - nodeLengthCoordinate[startIndex]) / refinementDistance) + (double(endIndex) - double(startIndex)));
    const auto& [outerStart, outerEnd] = m_outer_polygons_indices[polygonIndex];
    const auto numNodesNotRefinedPart = startIndex - outerStart + outerEnd - endIndex;
    const auto totalNumNodes = numNodesRefinedPart + numNodesNotRefinedPart;
    std::vector<Point> refinedPolygon;
    refinedPolygon.reserve(totalNumNodes);

    // before refinement
    for (auto i = outerStart; i <= startIndex; ++i)
    {
        refinedPolygon.emplace_back(m_nodes[i]);
    }

    // refined part
    auto nodeIndex = startIndex;
    auto nextNodeIndex = nodeIndex + 1;
    Point p0 = m_nodes[nodeIndex];
    Point p1 = m_nodes[nextNodeIndex];
    double pointLengthCoordinate = nodeLengthCoordinate[startIndex];
    bool snappedToLastPoint = false;
    while (nodeIndex < endIndex)
    {
        // initial point already accounted for
        pointLengthCoordinate += refinementDistance;
        if (pointLengthCoordinate > nodeLengthCoordinate[nextNodeIndex])
        {
            // if not snapped to the original last polygon point, snap it
            if (!snappedToLastPoint)
            {
                refinedPolygon.emplace_back(m_nodes[nextNodeIndex]);
            }

            // find the next point
            bool nextNodeFound = false;
            for (auto i = nextNodeIndex + 1; i <= endIndex; ++i)
            {
                if (nodeLengthCoordinate[i] > pointLengthCoordinate)
                {
                    nextNodeFound = true;
                    nodeIndex = i - 1;
                    nextNodeIndex = i;
                    break;
                }
            }
            if (nextNodeIndex > endIndex || !nextNodeFound)
            {
                break;
            }

            p0 = m_nodes[nodeIndex];
            p1 = m_nodes[nextNodeIndex];
            pointLengthCoordinate = nodeLengthCoordinate[nodeIndex] + refinementDistance;
            snappedToLastPoint = false;
        }
        double distanceFromLastNode = pointLengthCoordinate - nodeLengthCoordinate[nodeIndex];
        const double factor = distanceFromLastNode / edgeLengths[nodeIndex];
        Point p;
        if (IsEqual(factor, 1.0))
        {
            snappedToLastPoint = true;
            p = p1;
        }
        else
        {
            p = p0 + (p1 - p0) * distanceFromLastNode / edgeLengths[nodeIndex];
        }
        refinedPolygon.emplace_back(p);
    }

    // after refinement
    for (auto i = endIndex + 1; i <= outerEnd; ++i)
    {
        refinedPolygon.emplace_back(m_nodes[i]);
    }

    return refinedPolygon;
}

Polygons Polygons::OffsetCopy(double distance, bool innerAndOuter) const
{
    auto sizenewPolygon = GetNumNodes();
    if (innerAndOuter)
    {
        sizenewPolygon += GetNumNodes() + 1;
    }

    std::vector<Point> normalVectors(sizenewPolygon);
    double dxNormalPreviousEdge = 0.0;
    double dyNormalPreviousEdge = 0.0;
    double dxNormal = 0.0;
    double dyNormal = 0.0;
    for (size_t n = 0; n < GetNumNodes(); n++)
    {
        if (n < GetNumNodes() - 1)
        {
            const auto dx = GetDx(m_nodes[n], m_nodes[n + 1], m_projection);
            const auto dy = GetDy(m_nodes[n], m_nodes[n + 1], m_projection);
            const auto nodeDistance = std::sqrt(dx * dx + dy * dy);
            dxNormal = -dy / nodeDistance;
            dyNormal = dx / nodeDistance;
        }
        else
        {
            dxNormal = dxNormalPreviousEdge;
            dyNormal = dyNormalPreviousEdge;
        }

        if (n == 0)
        {
            dxNormalPreviousEdge = dxNormal;
            dyNormalPreviousEdge = dyNormal;
        }

        const double factor = 1.0 / (1.0 + dxNormalPreviousEdge * dxNormal + dyNormalPreviousEdge * dyNormal);
        normalVectors[n].x = factor * (dxNormalPreviousEdge + dxNormal);
        normalVectors[n].y = factor * (dyNormalPreviousEdge + dyNormal);

        dxNormalPreviousEdge = dxNormal;
        dyNormalPreviousEdge = dyNormal;
    }

    // negative sign introduced because normal vector pointing inward
    distance = -distance;
    if (m_projection == Projection::spherical)
    {
        distance = distance / (constants::geometric::earth_radius * constants::conversion::degToRad);
    }

    std::vector<Point> newPolygonPoints(sizenewPolygon, {constants::missing::doubleValue, constants::missing::doubleValue});
    for (size_t i = 0; i < GetNumNodes(); ++i)
    {
        auto dx = normalVectors[i].x * distance;
        const auto dy = normalVectors[i].y * distance;
        if (m_projection == Projection::spherical)
        {
            dx = dx / std::cos((m_nodes[i].y + 0.5 * dy) * constants::conversion::degToRad);
        }
        newPolygonPoints[i].x = m_nodes[i].x + dx;
        newPolygonPoints[i].y = m_nodes[i].y + dy;

        if (innerAndOuter)
        {
            newPolygonPoints[i + GetNumNodes() + 1].x = m_nodes[i].x - dx;
            newPolygonPoints[i + GetNumNodes() + 1].y = m_nodes[i].y - dy;
        }
    }

    // set the new polygon
    Polygons newPolygon{newPolygonPoints, m_projection};
    return newPolygon;
}

bool Polygons::IsPointInPolygon(Point const& point, size_t polygonIndex) const
{
    if (IsEmpty())
    {
        return true;
    }

    if (polygonIndex >= GetNumPolygons())
    {
        throw std::invalid_argument("Polygons::IsPointInPolygon: Invalid polygon index.");
    }
    const auto& [outerStart, outerEnd] = m_outer_polygons_indices[polygonIndex];
    const auto inPolygon = IsPointInPolygonNodes(point, m_nodes, m_projection, Point(), outerStart, outerEnd);
    return inPolygon;
}

size_t Polygons::GetNumPolygons() const
{
    return m_outer_polygons_indices.size();
}

std::tuple<bool, size_t> Polygons::IsPointInPolygons(Point point) const
{
    // empty polygon means everything is included
    if (m_outer_polygons_indices.empty())
    {
        return {true, constants::missing::sizetValue};
    }

    bool inPolygon = false;
    for (size_t polygonIndex = 0; polygonIndex < GetNumPolygons(); ++polygonIndex)
    {
        const auto& [polygonStartIndex, polygonEndIndex] = m_outer_polygons_indices[polygonIndex];

        // Calculate the bounding box
        double XMin = std::numeric_limits<double>::max();
        double XMax = std::numeric_limits<double>::lowest();
        double YMin = std::numeric_limits<double>::max();
        double YMax = std::numeric_limits<double>::lowest();

        for (auto n = polygonStartIndex; n <= polygonEndIndex; n++)
        {
            XMin = std::min(XMin, m_nodes[n].x);
            XMax = std::max(XMax, m_nodes[n].x);
            YMin = std::min(YMin, m_nodes[n].y);
            YMax = std::max(YMax, m_nodes[n].y);
        }

        if (point.x >= XMin && point.x <= XMax && (point.y >= YMin && point.y <= YMax))
        {
            inPolygon = IsPointInPolygonNodes(point, m_nodes, m_projection, Point(), polygonStartIndex, polygonEndIndex);
        }

        if (inPolygon)
        {
            for (const auto& [startInner, endInner] : m_inner_polygons_indices.at(polygonIndex))
            {
                if (IsPointInPolygonNodes(point, m_nodes, m_projection, Point(), startInner, endInner))
                {
                    return {false, constants::missing::sizetValue};
                }
            }
            return {true, polygonIndex};
        }
    }

    return {false, constants::missing::sizetValue};
}

std::vector<bool> Polygons::PointsInPolygons(const std::vector<Point>& points) const
{
    std::vector<bool> result(points.size(), false);
    for (size_t i = 0; i < points.size(); ++i)
    {
        const auto [isInPolygon, polygonIndex] = IsPointInPolygons(points[i]);
        result[i] = isInPolygon;
    }
    return result;
}

bool Polygons::IsEmpty() const
{
    return m_outer_polygons_indices.empty();
}

double Polygons::PerimeterClosedPolygon(const std::vector<Point>& polygonNodes) const
{
    if (polygonNodes.front() != polygonNodes.back())
    {
        throw std::invalid_argument("Polygons::PerimeterClosedPolygon: The first and last point of the polygon is not the same.");
    }

    const auto edgeLengths = PolygonEdgeLengths(polygonNodes);
    return std::accumulate(edgeLengths.begin(), edgeLengths.end(), 0.0);
}

std::vector<double> Polygons::PolygonEdgeLengths(const std::vector<Point>& polygonNodes) const
{
    std::vector<double> edgeLengths;
    edgeLengths.reserve(polygonNodes.size());

    for (size_t p = 0; p < polygonNodes.size(); ++p)
    {
        const auto firstNode = p;
        auto secondNode = p + 1;
        if (secondNode == polygonNodes.size())
        {
            secondNode = 0;
        }
        edgeLengths.emplace_back(ComputeDistance(polygonNodes[firstNode], polygonNodes[secondNode], m_projection));
    }
    return edgeLengths;
}

double Polygons::MaximumEdgeLength(const std::vector<Point>& polygonNodes) const
{

    if (polygonNodes.front() != polygonNodes.back())
    {
        throw std::invalid_argument("Polygons::MaximumEdgeLength: The first and last point of the polygon is not the same.");
    }

    auto maximumEdgeLength = std::numeric_limits<double>::lowest();
    for (size_t p = 0; p < polygonNodes.size() - 1; ++p)
    {
        double edgeLength = ComputeDistance(m_nodes[p], m_nodes[p + 1], m_projection);
        maximumEdgeLength = std::max(maximumEdgeLength, edgeLength);
    }
    return maximumEdgeLength;
}