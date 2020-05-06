#pragma once

#include <cmath>
#include <algorithm>
#include <numeric>
#include "Entities.hpp"
#include "Constants.cpp"
#include "SpatialTrees.hpp"

namespace GridGeom
{

    // coordinate reference independent operations
    template<typename T>
    T DotProduct(const T& dx1, const T& dx2)
    {
        return dx1 * dx2;
    }

    template<typename T, typename... Args>
    T DotProduct(const T& dx1, const T& dx2, Args&... args)
    {
        return dx1 * dx2 + DotProduct(args...);
    }


    template<typename T>
    bool ResizeVectorIfNeeded(int newSize, std::vector<T>& vectorToResize, T fillValue = T())
    {
        const int currentSize = vectorToResize.size();
        if (newSize > currentSize)
        {
            newSize = std::max(newSize, int(currentSize * 1.2));
            vectorToResize.resize(newSize, fillValue);
        }
        return true;
    }

    template<typename T>
    bool ResizeVectorIfNeededWithMinimumSize(int newSize, std::vector<T>& vectorToResize, int minSize, T fillValue = T())
    {
        const int currentSize = vectorToResize.size();
        if (newSize > currentSize)
        {
            newSize = std::max(minSize, int(5 * newSize));
            vectorToResize.resize(newSize, fillValue);
        }
        return true;
    }

    template <typename T>
    T FindIndex(const std::vector<T>& vec, const T& el)
    {
        T index = 0;
        for (int n = 0; n < vec.size(); n++)
        {
            if (vec[n] == el)
            {
                index = n;
                break;
            }
        }
        return index;
    }

    static int FindIndexes(const std::vector<Point>& vec, const int start, const int end, const double& separator, std::vector<std::vector<int>>& indexes)
    {
        // set an invalid index
        for (int n = 0; n < indexes.size(); n++)
        {
            indexes[n][0] = -1;
            indexes[n][1] = -1;
        }

        if (start > vec.size() || end > vec.size() || indexes.size() == 0)
        {
            return -1;

        }

        int pos = 0;
        for (int n = start; n < end; n++)
        {

            if (vec[n].x != separator && indexes[pos][0] == -1)
            {
                indexes[pos][0] = n;
            }
            else if (vec[n].x == separator && indexes[pos][1] == -1)
            {
                indexes[pos][1] = n - 1;
                pos++;
            }
            if (pos >= indexes.size())
            {
                return -1;
            }
        }

        if (pos < indexes.size() && indexes[pos][1] == -1)
        {
            indexes[pos][1] = int(vec.size()) - 1;
            pos++;
        }

        return pos;
    }

    template <typename T>
    inline std::vector<int> SortedIndexes(const std::vector<T>& v)
    {
        std::vector<int> idx(v.size());
        iota(idx.begin(), idx.end(), 0);
        std::stable_sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) {return v[i1] < v[i2]; });
        return std::move(idx);
    }

    //chmike's algorithm
    template< class T >
    void ReorderVector(std::vector<T>& v, std::vector<int> const& order)
    {
        std::vector<T> ordered(v.size());
        for (int i = 0; i < order.size(); ++i)
        {
            ordered[i] = v[order[i]];
        }
        v = ordered;
    }

    template<typename T>
    bool MakeMonothonic(std::vector<T>& v)
    {

        bool isMonotonic = false;
        int maxIter = 10;
        int iter = 0;
        while (!isMonotonic && iter < maxIter)
        {
            isMonotonic = true;
            maxIter++;
            for (int n = 0; n < v.size(); ++n)
            {
                if (v[n + 1] - v[n] < 0.0)
                {
                    isMonotonic = false;
                    break;
                }
            }
            if (!isMonotonic)
            {
                for (int n = 1; n < v.size() - 1; ++n)
                {
                    v[n] = 0.5 * (v[n - 1] + v[n + 1]);
                }
            }
        }
        return true;
    }

    template <typename T>
    void AddValueToVector(std::vector<T>& vec, const T value)
    {
        for (auto& val : vec)
        {
            val += value;
        }
    }


    // algorithm performing the zero's search using the golden section algorithm's
    template <typename F>
    double FindFunctionRootWithGoldenSectionSearch(F func, double min, double max)
    {
        //golden distance factors
        const double c = 0.38196602;
        const double r = 0.61803399;
        const double tolerance = 0.00001;

        double left = min;
        double middle = (min + max) * 0.5;
        double right = max;

        double x0 = left;
        double x1 = middle - c * (middle - left);
        double x2 = middle;
        double x3 = right;
        if (std::abs(right - middle) > std::abs(middle - left))
        {
            x1 = middle;
            x2 = middle + c * (middle - left);
        }

        double f1 = func(x1);
        double f2 = func(x2);

        while (std::abs(x3 - x0) > tolerance* std::max(std::abs(x1) + std::abs(x2), 1e-8))
        {
            if (f2 < f1)
            {
                x0 = x1;
                x1 = x2;
                x2 = r * x1 + c * x3;

                f1 = f2;
                f2 = func(x2);
            }
            else
            {
                x3 = x2;
                x2 = x1;
                x1 = r * x2 + c * x0;

                f2 = f1;
                f1 = func(x1);
            }
        }

        return f1 < f2 ? x1 : x2;
    }

    static bool IsPointOnPole(const Point& point)
    {
        return std::abs(point.y) - 90.0 < absLatitudeAtPoles;
    }

    ///sphertocart3D transform 2d spherical to 3d cartesian
    static void SphericalToCartesian(const Point& sphericalPoint, cartesian3DPoint& cartesianPoint)
    {
        cartesianPoint.z = earth_radius * sin(sphericalPoint.y * degrad_hp);
        double rr = earth_radius * cos(sphericalPoint.y * degrad_hp);
        cartesianPoint.x = rr * cos(sphericalPoint.x * degrad_hp);
        cartesianPoint.y = rr * sin(sphericalPoint.x * degrad_hp);
    }

    ///Cart3Dtospher Transform 3d cartesian coordinates to 2d spherical
    static void CartesianToSpherical(const cartesian3DPoint& cartesianPoint, double referenceLongitude, Point& sphericalPoint)
    {
        double angle = atan2(cartesianPoint.y, cartesianPoint.x) * raddeg_hp;
        sphericalPoint.y = atan2(cartesianPoint.z, sqrt(cartesianPoint.x * cartesianPoint.x + cartesianPoint.y * cartesianPoint.y)) * raddeg_hp;
        sphericalPoint.x = angle + std::lround((referenceLongitude - angle) / 360.0) * 360.0;
    }

    /// IsLeft(): tests if a point is Left|On|Right of an infinite line.
    ///    Input:  three points leftPoint, rightPoint, and point
    ///    Return: >0 for point left of the line through leftPoint and rightPoint
    ///            =0 for point  on the line
    ///            <0 for point  right of the line
    static double IsLeft(const Point& leftPoint, const Point& rightPoint, const Point& point)
    {
        double left = (rightPoint.x - leftPoint.x) * (point.y - leftPoint.y) - (point.x - leftPoint.x) * (rightPoint.y - leftPoint.y);
        return left;
    }


    static bool IsOnLine(const Point& leftPoint, const Point& rightPoint, const Point& point)
    {
        const double tolerance = 1e-8;
        const double crossProduct = (point.y - leftPoint.y) * (rightPoint.x - leftPoint.x) - (point.x - leftPoint.x) * (rightPoint.y - leftPoint.y);

        // it is not aligned
        if (crossProduct > tolerance)
        {
            return false;
        }

        const double dotProduct = (point.x - leftPoint.x) * (rightPoint.x - leftPoint.x) + (point.y - leftPoint.y) * (rightPoint.y - leftPoint.y);

        // it is not between left and right
        if (dotProduct < 0)
        {
            return false;
        }

        double const squaredDistance = (rightPoint.x - leftPoint.x) * (rightPoint.x - leftPoint.x) + (rightPoint.y - leftPoint.y) * (rightPoint.y - leftPoint.y);

        if (dotProduct > squaredDistance)
        {
            return false;
        }

        return true;

    }

    /// Check if a point is in polygonNodes using the winding number method
    /// polygonNodes: a closed polygonNodes consisting f a vector of numberOfPolygonPoints + 1 in counter clockwise order
    static bool IsPointInPolygonNodes(const Point& point, const std::vector<Point>& polygonNodes, int startNode, int endNode)
    {
        if (endNode <= startNode)
        {
            return true;
        }
        const int currentPolygonSize = endNode - startNode + 1;
        if (polygonNodes.size() < currentPolygonSize)
        {
            return false;
        }
        if (polygonNodes[startNode] != polygonNodes[endNode])
        {
            return false;
        }

        int windingNumber = 0;
        for (int n = startNode; n < endNode; n++)
        {
            if (polygonNodes[n].y <= point.y) // an upward crossing
            {
                if (polygonNodes[n + 1].y >= point.y && IsLeft(polygonNodes[n], polygonNodes[n + 1], point) >= 0.0)
                {
                    ++windingNumber; // have  a valid up intersect
                }
            }
            else
            {
                if (polygonNodes[n + 1].y <= point.y && IsLeft(polygonNodes[n], polygonNodes[n + 1], point) <= 0.0) // a downward crossing
                {
                    --windingNumber; // have  a valid down intersect
                }

            }
        }
        return windingNumber == 0 ? false : true;
    }

    static double GetDx(const Point& firstPoint, const Point& secondPoint, const Projections& projection)
    {
        if (projection == Projections::cartesian)
        {
            return secondPoint.x - firstPoint.x;
        }
        if (projection == Projections::spherical)
        {
            bool  isFirstPointOnPole = IsPointOnPole(firstPoint);
            bool  isSecondPointOnPole = IsPointOnPole(secondPoint);
            if (isFirstPointOnPole && !isSecondPointOnPole || !isFirstPointOnPole && isSecondPointOnPole)
            {
                return 0.0;
            }
            double firstPointX = firstPoint.x;
            double secondPointX = secondPoint.x;
            if (firstPointX - secondPointX > 180.0)
            {
                firstPointX -= 360.0;
            }
            else if (firstPointX - secondPointX < -180.0)
            {
                firstPointX += 360.0;
            }

            firstPointX = firstPointX * degrad_hp;
            secondPointX = secondPointX * degrad_hp;
            double firstPointY = firstPoint.y * degrad_hp;
            double secondPointY = secondPoint.y * degrad_hp;
            double cosPhi = cos(0.5 * (firstPointY + secondPointY));
            double dx = earth_radius * cosPhi * (secondPointX - firstPointX);
            return dx;
        }
        return doubleMissingValue;
    }

    static double GetDy(const Point& firstPoint, const Point& secondPoint, const Projections& projection)
    {
        if (projection == Projections::cartesian)
        {
            return secondPoint.y - firstPoint.y;
        }
        if (projection == Projections::spherical)
        {
            double firstPointY = firstPoint.y * degrad_hp;
            double secondPointY = secondPoint.y * degrad_hp;
            double dy = earth_radius * (secondPointY - firstPointY);
            return dy;
        }
        return doubleMissingValue;
    }

    ///dprodout: out product of two segments
    static double OuterProductTwoSegments(const Point& firstPointFirstSegment, const Point& secondPointFirstSegment,
        const Point& firstPointSecondSegment, const Point& secondPointSecondSegment, const Projections& projection)
    {
        if (projection == Projections::cartesian)
        {
            double dx1 = GetDx(firstPointFirstSegment, secondPointFirstSegment, projection);
            double dx2 = GetDx(firstPointSecondSegment, secondPointSecondSegment, projection);

            double dy1 = GetDy(firstPointFirstSegment, secondPointFirstSegment, projection);
            double dy2 = GetDy(firstPointSecondSegment, secondPointSecondSegment, projection);

            return dx1 * dy2 - dy1 * dx2;
        }
        if (projection == Projections::spherical)
        {
            cartesian3DPoint firstPointFirstSegmentCartesian;
            SphericalToCartesian(firstPointFirstSegment, firstPointFirstSegmentCartesian);
            auto xx1 = firstPointFirstSegmentCartesian.x;
            auto yy1 = firstPointFirstSegmentCartesian.y;
            auto zz1 = firstPointFirstSegmentCartesian.z;

            cartesian3DPoint secondPointFirstSegmentCartesian;
            SphericalToCartesian(secondPointFirstSegment, secondPointFirstSegmentCartesian);
            auto xx2 = secondPointFirstSegmentCartesian.x;
            auto yy2 = secondPointFirstSegmentCartesian.y;
            auto zz2 = secondPointFirstSegmentCartesian.z;

            cartesian3DPoint firstPointSecondSegmentCartesian;
            SphericalToCartesian(firstPointSecondSegment, firstPointSecondSegmentCartesian);
            auto xx3 = firstPointSecondSegmentCartesian.x;
            auto yy3 = firstPointSecondSegmentCartesian.y;
            auto zz3 = firstPointSecondSegmentCartesian.z;


            cartesian3DPoint secondPointSecondSegmentCartesian;
            SphericalToCartesian(secondPointSecondSegment, secondPointSecondSegmentCartesian);
            auto xx4 = secondPointSecondSegmentCartesian.x;
            auto yy4 = secondPointSecondSegmentCartesian.y;
            auto zz4 = secondPointSecondSegmentCartesian.z;

            double vxx = (yy2 - yy1) * (zz4 - zz3) - (zz2 - zz1) * (yy4 - yy3);
            double vyy = (zz2 - zz1) * (xx4 - xx3) - (xx2 - xx1) * (zz4 - zz3);
            double vzz = (xx2 - xx1) * (yy4 - yy3) - (yy2 - yy1) * (xx4 - xx3);

            double result = std::sqrt(vxx * vxx + vyy * vyy + vzz * vzz);

            //check if vector is pointing outwards of earth
            if (vxx * xx1 + vyy * yy1 + vzz * zz1 < 0.0)
            {
                result = -result;
            }
            return result;
        }
        return doubleMissingValue;
    }

    static void MiddlePoint(const Point& firstPoint, const Point& secondPoint, Point& result, const Projections& projection)
    {
        if (projection == Projections::cartesian)
        {
            cartesian3DPoint firstPointCartesianCoordinates;
            SphericalToCartesian(firstPoint, firstPointCartesianCoordinates);
            cartesian3DPoint secondPointCartesianCoordinates;
            SphericalToCartesian(secondPoint, secondPointCartesianCoordinates);

            cartesian3DPoint middleCartesianPointCoordinate;
            middleCartesianPointCoordinate.x = 0.5 * (firstPointCartesianCoordinates.x + secondPointCartesianCoordinates.x);
            middleCartesianPointCoordinate.y = 0.5 * (firstPointCartesianCoordinates.y + secondPointCartesianCoordinates.y);
            double referenceLongitude = std::max(firstPoint.x, secondPoint.x);

            CartesianToSpherical(middleCartesianPointCoordinate, referenceLongitude, result);
        }
        if (projection == Projections::spherical)
        {
            result = { (firstPoint.x + secondPoint.x) / 2.0, (firstPoint.y + secondPoint.y) / 2.0 };
        }
    }

    ///normaloutchk, Computes the normal vector to a line 1-2, which is *outward* w.r.t. an 'inside' point 3. Similar to normalout, except that the normal
    ///vector may be flipped based on the 'inside' point.
    static void NormalVector(const Point& firstPoint, const Point& secondPoint, const Point& insidePoint, Point& result, const Projections& projection)
    {
        if (projection == Projections::cartesian)
        {
            double dx = GetDx(firstPoint, secondPoint, projection);
            double dy = GetDy(firstPoint, secondPoint, projection);
            const double squaredDistance = dx * dx + dy * dy;
            if (squaredDistance != 0.0)
            {
                const double distance = sqrt(squaredDistance);
                result.x = dy / distance;
                result.y = -dx / distance;
            }
        }
        if (projection == Projections::spherical)
        {
            cartesian3DPoint firstPointCartesianCoordinates;
            cartesian3DPoint secondPointCartesianCoordinates;
            SphericalToCartesian(firstPoint, firstPointCartesianCoordinates);
            SphericalToCartesian(secondPoint, secondPointCartesianCoordinates);

            double lambda = insidePoint.x * degrad_hp;
            double phi = insidePoint.y * degrad_hp;
            double elambda[3] = { -sin(lambda), cos(lambda), 0.0 };
            double ephi[3] = { -sin(lambda), cos(lambda), 0.0 };

            double dx = (secondPointCartesianCoordinates.x - firstPointCartesianCoordinates.x) * elambda[0] +
                (secondPointCartesianCoordinates.y - firstPointCartesianCoordinates.y) * elambda[1] +
                (secondPointCartesianCoordinates.z - firstPointCartesianCoordinates.z) * elambda[2];

            double dy = (secondPointCartesianCoordinates.x - firstPointCartesianCoordinates.x) * ephi[0] +
                (secondPointCartesianCoordinates.y - firstPointCartesianCoordinates.y) * ephi[1] +
                (secondPointCartesianCoordinates.z - firstPointCartesianCoordinates.z) * ephi[2];

            double squaredDistance = dx * dx + dy * dy;
            if (squaredDistance != 0.0)
            {
                const double distance = sqrt(squaredDistance);
                result.x = dx / distance;
                result.y = dy / distance;
            }
        }
    }

    ///normaloutchk
    static void NormalVectorInside(const Point& firstPoint, const Point& secondPoint, const Point& insidePoint, Point& result, bool& flippedNormal, const Projections& projection)
    {
        if (projection == Projections::cartesian)
        {
            NormalVector(firstPoint, secondPoint, insidePoint, result, projection);
            flippedNormal = false;
            Point thirdPoint{ firstPoint.x + result.x, firstPoint.y + result.y };

            if (OuterProductTwoSegments(firstPoint, thirdPoint, firstPoint, secondPoint, projection) * OuterProductTwoSegments(firstPoint, insidePoint, firstPoint, secondPoint, projection) > 0.0)
            {
                result.x = -result.x;
                result.y = -result.y;
                flippedNormal = true;
            }
            else
            {
                flippedNormal = false;
            }
        }
        if (projection == Projections::spherical)
        {
            //if (JSFERIC.eq.1 . and .jasfer3D.eq.0) xn = xn * cos(dg2rd * 0.5d0 * (y0 + y1)) !normal vector needs to be in Cartesian coordinates
        }
    }

    ///normalout
    static void NormalVectorOutside(const Point& firstPoint, const Point& secondPoint, Point& result, const Projections& projection)
    {
        double dx = 0.0;
        double dy = 0.0;
        if (projection == Projections::cartesian)
        {
            dx = GetDx(firstPoint, secondPoint, projection);
            dy = GetDy(firstPoint, secondPoint, projection);

            const double squaredDistance = dx * dx + dy * dy;
            if (squaredDistance > 0.0)
            {
                const double distance = sqrt(squaredDistance);
                result.x = dy / distance;
                result.y = -dx / distance;
            }
            else
            {
                result.x = doubleMissingValue;
                result.y = doubleMissingValue;
            }
        }
        if (projection == Projections::spherical)
        {
            Point middlePoint;
            MiddlePoint(firstPoint, secondPoint, middlePoint, projection);

            cartesian3DPoint firstPointCartesianCoordinates;
            SphericalToCartesian(firstPoint, firstPointCartesianCoordinates);
            cartesian3DPoint secondPointCartesianCoordinates;
            SphericalToCartesian(secondPoint, secondPointCartesianCoordinates);

            //compute the base vectors at middle point
            double lambda = middlePoint.x * degrad_hp;
            double phi = middlePoint.y * degrad_hp;
            double elambda[3] = { -sin(lambda), cos(lambda), 0.0 };
            double ephi[3] = { -sin(phi) * cos(lambda), -sin(phi) * sin(lambda), cos(phi) };

            // project vector in local base
            dx = (secondPointCartesianCoordinates.x - firstPointCartesianCoordinates.x) * elambda[0] +
                (secondPointCartesianCoordinates.y - firstPointCartesianCoordinates.y) * elambda[1] +
                (secondPointCartesianCoordinates.z - firstPointCartesianCoordinates.z) * elambda[2];

            dy = (secondPointCartesianCoordinates.x - firstPointCartesianCoordinates.x) * ephi[0] +
                (secondPointCartesianCoordinates.y - firstPointCartesianCoordinates.y) * ephi[1] +
                (secondPointCartesianCoordinates.z - firstPointCartesianCoordinates.z) * ephi[2];

            const double squaredDistance = dx * dx + dy * dy;
            if (squaredDistance > 0.0)
            {
                const double distance = sqrt(squaredDistance);
                result.x = dy / distance;
                result.y = -dx / distance;
            }
            else
            {
                result.x = doubleMissingValue;
                result.y = doubleMissingValue;
            }

            result.x = result.x / cos(degrad_hp * 0.5 * (firstPoint.y + secondPoint.y));
            result.y = middlePoint.y;
        }

    }

    static void Add(Point& point, const Point& normal, const double increment, const Projections& projection)
    {
        if (projection == Projections::cartesian)
        {
            point.x = point.x + normal.x * increment;
            point.y = point.y + normal.y * increment;
        }
        if (projection == Projections::spherical)
        {
            double convertedIncrement = raddeg_hp * increment / earth_radius;
            double xf = 1.0 / cos(degrad_hp * point.y);
            point.x = point.x + normal.x * convertedIncrement * xf;
            point.y = point.y + normal.y * convertedIncrement;
        }
    }

    static void ReferencePoint(std::vector<Point>& polygon, const int numPoints, double& minX, double& minY, const Projections& projection)
    {
        if (projection == Projections::cartesian)
        {
            minX = std::numeric_limits<double>::max();
            minY = std::numeric_limits<double>::max();
            for (int i = 0; i < numPoints; i++)
            {
                if (polygon[i].x < minX)
                {
                    minX = polygon[i].x;
                }
                if (abs(polygon[i].y) < abs(minY))
                {
                    minY = polygon[i].y;
                }
            }
        }
        if (projection == Projections::spherical)
        {
            minX = std::numeric_limits<double>::max();
            minY = std::numeric_limits<double>::max();
            double xmax = std::numeric_limits<double>::min();
            for (int i = 0; i < numPoints; i++)
            {
                if (polygon[i].x < minX)
                {
                    minX = polygon[i].x;
                }
                if (abs(polygon[i].y) < abs(minY))
                {
                    minY = polygon[i].y;
                }
                if (polygon[i].x > xmax)
                {
                    xmax = polygon[i].x;
                }
            }

            if (xmax - minX > 180.0)
            {
                double deltaX = xmax - 180.0;
                for (int i = 0; i < numPoints; i++)
                {
                    if (polygon[i].x < deltaX)
                    {
                        polygon[i].x = polygon[i].x + 360.0;
                    }
                }
                minX = minX + 360.0;
            }
            minX = std::min_element(polygon.begin(), polygon.end(), [](const Point& p1, const Point& p2) { return p1.x < p2.x; })->x;
        }
    }

    static double SquaredDistance(const Point& firstPoint, const Point& secondPoint, const Projections& projection)
    {

        if (firstPoint.x == doubleMissingValue || firstPoint.y == doubleMissingValue ||
            secondPoint.x == doubleMissingValue || secondPoint.y == doubleMissingValue)
            return 0.0;

        if (projection == Projections::cartesian)
        {
            double dx = GetDx(firstPoint, secondPoint, projection);
            double dy = GetDy(firstPoint, secondPoint, projection);
            return dx * dx + dy * dy;
        }
        if (projection == Projections::spherical)
        {
            cartesian3DPoint firstPointCartesian;
            SphericalToCartesian(firstPoint, firstPointCartesian);
            auto xx1 = firstPointCartesian.x;
            auto yy1 = firstPointCartesian.y;
            auto zz1 = firstPointCartesian.z;

            cartesian3DPoint secondPointCartesian;
            SphericalToCartesian(secondPoint, secondPointCartesian);
            auto xx2 = secondPointCartesian.x;
            auto yy2 = secondPointCartesian.y;
            auto zz2 = secondPointCartesian.z;

            return (xx2 - xx1) * (xx2 - xx1) + (yy2 - yy1) * (yy2 - yy1) + (zz2 - zz1) * (zz2 - zz1);
        }

        return doubleMissingValue;
    }

    //dbdistance
    static double Distance(const Point& firstPoint, const Point& secondPoint, const Projections& projection)
    {
        double distance = SquaredDistance(firstPoint, secondPoint, projection);
        if (distance >= 0.0)
        {
            distance = sqrt(distance);
        }
        return distance;
    }

    // dLINEDIS3
    // Computes the perpendicular distance from point to a line firstNode - secondNode.
    // normalPoint: coordinates of the projected point from point onto the line
    static double DistanceFromLine(const Point& point, const Point& firstNode, const Point& secondNode, Point& normalPoint, double& ratio, const Projections& projection)
    {
        if (projection == Projections::cartesian)
        {
            double dis = 0.0;
            double squaredDistance = SquaredDistance(secondNode, firstNode, projection);
            if (squaredDistance != 0.0)
            {
                ratio = (GetDx(firstNode, point, projection) * GetDx(firstNode, secondNode, projection) +
                    GetDy(firstNode, point, projection) * GetDy(firstNode, secondNode, projection)) / squaredDistance;
                double correctedRatio = std::max(std::min(1.0, ratio), 0.0);
                normalPoint.x = firstNode.x + correctedRatio * (secondNode.x - firstNode.x);
                normalPoint.y = firstNode.y + correctedRatio * (secondNode.y - firstNode.y);
                dis = Distance(point, normalPoint, projection);
            }
            return dis;
        }
        if (projection == Projections::spherical)
        {
            cartesian3DPoint firstNodeCartesian;
            SphericalToCartesian(firstNode, firstNodeCartesian);
            auto xx1 = firstNodeCartesian.x;
            auto yy1 = firstNodeCartesian.y;
            auto zz1 = firstNodeCartesian.z;

            cartesian3DPoint secondNodeCartesian;
            SphericalToCartesian(secondNode, secondNodeCartesian);
            auto xx2 = secondNodeCartesian.x;
            auto yy2 = secondNodeCartesian.y;
            auto zz2 = secondNodeCartesian.z;

            cartesian3DPoint pointCartesian;
            SphericalToCartesian(point, pointCartesian);
            auto xx3 = pointCartesian.x;
            auto yy3 = pointCartesian.y;
            auto zz3 = pointCartesian.z;

            double x21 = xx2 - xx1;
            double y21 = yy2 - yy1;
            double z21 = zz2 - zz1;
            double x31 = xx3 - xx1;
            double y31 = yy3 - yy1;
            double z31 = zz3 - zz1;

            double r2 = x21 * x21 + y21 * y21 + z21 * z21;

            ratio = 0.0;
            if (r2 >= 0.0)
            {

                ratio = (x31 * x21 + y31 * y21 + z31 * z21) / r2;
                double correctedRatio = std::max(std::min(1.0, ratio), 0.0);

                cartesian3DPoint cartesianNormal3DPoint;
                cartesianNormal3DPoint.x = firstNodeCartesian.x + correctedRatio * x21;
                cartesianNormal3DPoint.y = firstNodeCartesian.y + correctedRatio * y21;
                cartesianNormal3DPoint.z = firstNodeCartesian.z + correctedRatio * z21;

                cartesianNormal3DPoint.x = cartesianNormal3DPoint.x - xx3;
                cartesianNormal3DPoint.y = cartesianNormal3DPoint.y - yy3;
                cartesianNormal3DPoint.z = cartesianNormal3DPoint.z - zz3;

                double dis = std::sqrt(cartesianNormal3DPoint.x * cartesianNormal3DPoint.x +
                    cartesianNormal3DPoint.y * cartesianNormal3DPoint.y +
                    cartesianNormal3DPoint.z * cartesianNormal3DPoint.z);

                double referenceLongitude = std::max({ firstNode.x, secondNode.x,point.x });
                CartesianToSpherical(cartesianNormal3DPoint, referenceLongitude, normalPoint);

                return dis;
            }
        }
        return -1.0;
    }

    /// dprodin inner product of two segments
    static double InnerProductTwoSegments(const Point& firstPointFirstSegment, const Point& secondPointFirstSegment, const Point& firstPointSecondSegment, const Point& secondPointSecondSegment, const Projections& projection)
    {
        if (projection == Projections::cartesian)
        {
            double dx1 = GetDx(firstPointFirstSegment, secondPointFirstSegment, projection);
            double dx2 = GetDx(firstPointSecondSegment, secondPointSecondSegment, projection);

            double dy1 = GetDy(firstPointFirstSegment, secondPointFirstSegment, projection);
            double dy2 = GetDy(firstPointSecondSegment, secondPointSecondSegment, projection);

            return dx1 * dx2 + dy1 * dy2;
        }
        if (projection == Projections::spherical)
        {
            cartesian3DPoint firstPointFirstSegment3D;
            cartesian3DPoint secondPointFirstSegment3D;
            cartesian3DPoint firstPointSecondSegment3D;
            cartesian3DPoint secondPointSecondSegment3D;

            SphericalToCartesian(firstPointFirstSegment, firstPointFirstSegment3D);
            SphericalToCartesian(secondPointFirstSegment, secondPointFirstSegment3D);
            SphericalToCartesian(firstPointSecondSegment, firstPointSecondSegment3D);
            SphericalToCartesian(secondPointSecondSegment, secondPointSecondSegment3D);

            double dx1 = secondPointFirstSegment3D.x - firstPointFirstSegment3D.x;
            double dy1 = secondPointFirstSegment3D.y - firstPointFirstSegment3D.y;
            double dz1 = secondPointFirstSegment3D.z - firstPointFirstSegment3D.z;

            double dx2 = secondPointSecondSegment3D.x - firstPointSecondSegment3D.x;
            double dy2 = secondPointSecondSegment3D.y - firstPointSecondSegment3D.y;
            double dz2 = secondPointSecondSegment3D.z - firstPointSecondSegment3D.z;

            return DotProduct(dx1, dx2, dy1, dy2, dz1, dz2);
        }

        return doubleMissingValue;
    }

    // dcosphi
    static double NormalizedInnerProductTwoSegments(const Point& firstPointFirstSegment, const Point& secondPointFirstSegment, const Point& firstPointSecondSegment, const Point& secondPointSecondSegment, const Projections& projection)
    {
        if (projection == Projections::cartesian)
        {
            const auto dx1 = GetDx(firstPointFirstSegment, secondPointFirstSegment, projection);
            const auto dx2 = GetDx(firstPointSecondSegment, secondPointSecondSegment, projection);

            const auto dy1 = GetDy(firstPointFirstSegment, secondPointFirstSegment, projection);
            const auto dy2 = GetDy(firstPointSecondSegment, secondPointSecondSegment, projection);

            const auto r1 = dx1 * dx1 + dy1 * dy1;
            const auto r2 = dx2 * dx2 + dy2 * dy2;

            double cosphi;
            if (r1 <= 0.0 || r2 <= 0.0)
            {
                cosphi = doubleMissingValue;
            }

            cosphi = (dx1 * dx2 + dy1 * dy2) / std::sqrt(r1 * r2);
            cosphi = std::max(std::min(cosphi, 1.0), -1.0);
            return cosphi;
        }
        if (projection == Projections::spherical)
        {
            cartesian3DPoint firstPointFirstSegmentCartesian;
            SphericalToCartesian(firstPointFirstSegment, firstPointFirstSegmentCartesian);
            auto xx1 = firstPointFirstSegmentCartesian.x;
            auto yy1 = firstPointFirstSegmentCartesian.y;
            auto zz1 = firstPointFirstSegmentCartesian.z;

            cartesian3DPoint secondPointFirstSegmentCartesian;
            SphericalToCartesian(secondPointFirstSegment, secondPointFirstSegmentCartesian);
            auto xx2 = secondPointFirstSegmentCartesian.x;
            auto yy2 = secondPointFirstSegmentCartesian.y;
            auto zz2 = secondPointFirstSegmentCartesian.z;

            cartesian3DPoint firstPointSecondSegmentCartesian;
            SphericalToCartesian(firstPointSecondSegment, firstPointSecondSegmentCartesian);
            auto xx3 = firstPointSecondSegmentCartesian.x;
            auto yy3 = firstPointSecondSegmentCartesian.y;
            auto zz3 = firstPointSecondSegmentCartesian.z;

            cartesian3DPoint secondPointSecondSegmentCartesian;
            SphericalToCartesian(secondPointSecondSegment, secondPointSecondSegmentCartesian);
            auto xx4 = secondPointSecondSegmentCartesian.x;
            auto yy4 = secondPointSecondSegmentCartesian.y;
            auto zz4 = secondPointSecondSegmentCartesian.z;

            auto dx1 = xx2 - xx1;
            auto dy1 = yy2 - yy1;
            auto dz1 = zz2 - zz1;
            auto firstSegmentDistance = dx1 * dx1 + dy1 * dy1 + dz1 * dz1;

            auto dx2 = xx4 - xx3;
            auto dy2 = yy4 - yy3;
            auto dz2 = zz4 - zz3;
            auto secondSegmentDistance = dx2 * dx2 + dy2 * dy2 + dz2 * dz2;

            double cosphi;
            if (firstSegmentDistance <= 0.0 || secondSegmentDistance <= 0.0)
            {
                cosphi = doubleMissingValue;
            }
            else
            {
                cosphi = (dx1 * dx2 + dy1 * dy2 + dz1 * dz2) / sqrt(firstSegmentDistance * secondSegmentDistance);
            }
            return cosphi;
        }
        return doubleMissingValue;
    }

    static bool OrthogonalizationComputeLocalCoordinates(const std::vector<int>& m_nodesNumEdges, const std::vector<int>& numConnectedNodes, std::vector<int>& localCoordinates, const Projections& projection)
    {
        if (projection == Projections::cartesian)
        {
            //do nothing
        }
        if (projection == Projections::spherical)
        {
            localCoordinates.resize(m_nodesNumEdges.size(), 0);
            localCoordinates[0] = 1;
            for (int i = 0; i < m_nodesNumEdges.size(); i++)
            {
                localCoordinates[i + 1] = localCoordinates[i] + std::max(m_nodesNumEdges[i] + 1, numConnectedNodes[i]);
            }
        }
        return true;
    }

    static bool orthogonalizationComputeJacobian(const int currentNode, const std::vector<double>& Jxi, const std::vector<double>& Jeta, const std::vector<std::size_t>& connectedNodes, const int numNodes, const std::vector<Point>& nodes, std::vector<double>& J, const Projections& projection)
    {
        if (projection == Projections::cartesian)
        {
            J[0] = 0.0;
            J[1] = 0.0;
            J[2] = 0.0;
            J[3] = 0.0;
            for (int i = 0; i < numNodes; i++)
            {
                J[0] += Jxi[i] * nodes[connectedNodes[i]].x;
                J[1] += Jxi[i] * nodes[connectedNodes[i]].y;
                J[2] += Jeta[i] * nodes[connectedNodes[i]].x;
                J[3] += Jeta[i] * nodes[connectedNodes[i]].y;
            }
        }
        if (projection == Projections::spherical)
        {
            double factor = std::cos(nodes[currentNode].y) * degrad_hp;
            J[0] = 0.0;
            J[1] = 0.0;
            J[2] = 0.0;
            J[3] = 0.0;
            for (int i = 0; i < numNodes; i++)
            {
                J[0] += Jxi[i] * nodes[connectedNodes[i]].x;
                J[1] += Jxi[i] * nodes[connectedNodes[i]].y;
                J[2] += Jeta[i] * nodes[connectedNodes[i]].x;
                J[3] += Jeta[i] * nodes[connectedNodes[i]].y;
            }
        }
        return true;
    }

    static bool orthogonalizationComputeDeltasDxDy(int firstNode, int secondNode, double wwx, double wwy, const std::vector<Point>& nodes, double& dx0, double& dy0, const Projections& projection)
    {
        if (projection == Projections::cartesian)
        {
            dx0 += wwx * (nodes[firstNode].x - nodes[secondNode].x);
            dy0 += wwy * (nodes[firstNode].y - nodes[secondNode].y);
        }
        if (projection == Projections::spherical)
        {
            double wwxTransformed = wwx * earth_radius * degrad_hp;
            double wwyTransformed = wwy * earth_radius * degrad_hp;

            dx0 += dx0 + wwxTransformed * (nodes[firstNode].x - nodes[secondNode].x);
            dy0 += dy0 + wwyTransformed * (nodes[firstNode].y - nodes[secondNode].y);
        }
        return true;
    }

    static bool orthogonalizationComputeIncrements(double wwx, double wwy, double* increments, const Projections& projection)
    {
        if (projection == Projections::cartesian)
        {
            increments[0] += wwx;
            increments[1] += wwy;

        }
        if (projection == Projections::spherical)
        {
            double wwxTransformed = wwx * earth_radius * degrad_hp;
            double wwyTransformed = wwy * earth_radius * degrad_hp;

            increments[0] += wwxTransformed;
            increments[1] += wwyTransformed;
        }
        return true;
    }

    static bool orthogonalizationComputeDeltas(int firstNode, int secondNode, double wwx, double wwy, const std::vector<Point>& nodes, double& dx0, double& dy0, double* increments, const Projections& projection)
    {
        if (projection == Projections::cartesian)
        {
            increments[0] += wwx;
            increments[1] += wwy;

            dx0 = dx0 + wwx * (nodes[firstNode].x - nodes[secondNode].x);
            dy0 = dy0 + wwy * (nodes[firstNode].y - nodes[secondNode].y);
        }
        if (projection == Projections::spherical)
        {
            double wwxTransformed = wwx * earth_radius * degrad_hp;
            double wwyTransformed = wwy * earth_radius * degrad_hp;

            increments[0] += wwxTransformed;
            increments[1] += wwyTransformed;

            dx0 = dx0 + wwxTransformed * (nodes[firstNode].x - nodes[secondNode].x);
            dy0 = dy0 + wwxTransformed * (nodes[firstNode].y - nodes[secondNode].y);
        }
        return true;
    }

    static bool orthogonalizationComputeCoordinates(double dx0, double dy0, const Point& point, Point& updatedPoint, const Projections& projection)
    {
        if (projection == Projections::cartesian)
        {
            double x0 = point.x + dx0;
            double y0 = point.y + dy0;
            static constexpr double relaxationFactorCoordinates = 1.0 - relaxationFactorOrthogonalizationUpdate;

            updatedPoint.x = relaxationFactorOrthogonalizationUpdate * x0 + relaxationFactorCoordinates * point.x;
            updatedPoint.y = relaxationFactorOrthogonalizationUpdate * y0 + relaxationFactorCoordinates * point.y;
        }
        if (projection == Projections::spherical)
        {
            //TODO: implement
            //if (jsferic.eq.1 . and .jasfer3D.eq.1) then
            //    dumx(1) = relaxin * Dx0
            //    dumy(1) = relaxin * Dy0
            //    call loc2spher(xk(k), yk(k), 1, dumx, dumy, xk1(k), yk1(k))
            //else
        }
        return true;
    }

    static bool CircumcenterOfTriangle(const Point& p1, const Point& p2, const Point& p3, const Projections projection, Point& circumcenter)
    {
        if (projection == Projections::cartesian)
        {
            double dx2 = GetDx(p1, p2, projection);
            double dy2 = GetDy(p1, p2, projection);

            double dx3 = GetDx(p1, p3, projection);
            double dy3 = GetDy(p1, p3, projection);

            double den = dy2 * dx3 - dy3 * dx2;
            double z = 0.0;
            if (den != 0.0)
            {
                z = (dx2 * (dx2 - dx3) + dy2 * (dy2 - dy3)) / den;
            }

            circumcenter.x = p1.x + 0.5 * (dx3 - z * dy3);
            circumcenter.y = p1.y + 0.5 * (dy3 + z * dx3);
        }
        if (projection == Projections::spherical)
        {
            double dx2 = GetDx(p1, p2, projection);
            double dy2 = GetDy(p1, p2, projection);

            double dx3 = GetDx(p1, p3, projection);
            double dy3 = GetDy(p1, p3, projection);

            double den = dy2 * dx3 - dy3 * dx2;
            double z = 0.0;
            if (den > 1e-16)
            {
                z = (dx2 * (dx2 - dx3) + dy2 * (dy2 - dy3)) / den;
            }

            //TODO circumcenter3 FINISH
            //phi = (y(1) + y(2) + y(3)) / 3d0
            //    xf = 1d0 / dcos(degrad_hp * phi)
            //    xz = x(1) + xf * 0.5d0 * (dx3 - z * dy3) * raddeg_hp / earth_radius
            //    yz = y(1) + 0.5d0 * (dy3 + z * dx3) * raddeg_hp / earth_radius
        }
        return true;
    }

    //CROSS
    static bool AreLinesCrossing(const Point& firstSegmentFistPoint,
        const Point& firstSegmentSecondPoint,
        const Point& secondSegmentFistPoint,
        const Point& secondSegmentSecondPoint,
        bool adimensional,
        Point& intersection,
        double& crossProduct,
        double& firstRatio,
        double& secondRatio,
        const Projections& projection)
    {
        bool isCrossing = false;
        if (projection == Projections::cartesian)
        {
            firstRatio = doubleMissingValue;
            secondRatio = doubleMissingValue;
            double x21 = GetDx(firstSegmentFistPoint, firstSegmentSecondPoint, projection);
            double y21 = GetDy(firstSegmentFistPoint, firstSegmentSecondPoint, projection);

            double x43 = GetDx(secondSegmentFistPoint, secondSegmentSecondPoint, projection);
            double y43 = GetDy(secondSegmentFistPoint, secondSegmentSecondPoint, projection);

            double x31 = GetDx(firstSegmentFistPoint, secondSegmentFistPoint, projection);
            double y31 = GetDy(firstSegmentFistPoint, secondSegmentFistPoint, projection);

            double det = x43 * y21 - y43 * x21;

            std::vector<double> values{ x21, y21, x43, y43 };
            double eps = std::max(0.00001 * (*std::max_element(values.begin(), values.end())), std::numeric_limits<double>::denorm_min());

            if (std::abs(det) < eps)
            {
                return isCrossing;
            }

            secondRatio = (y31 * x21 - x31 * y21) / det;
            firstRatio = (y31 * x43 - x31 * y43) / det;
            if (firstRatio >= 0.0 && firstRatio <= 1.0 && secondRatio >= 0.0 && secondRatio <= 1.0)
            {
                isCrossing = true;
            }
            intersection.x = firstSegmentFistPoint.x + firstRatio * (firstSegmentSecondPoint.x - firstSegmentFistPoint.x);
            intersection.y = firstSegmentFistPoint.y + firstRatio * (firstSegmentSecondPoint.y - firstSegmentFistPoint.y);
            crossProduct = -det;
            if (adimensional)
            {
                crossProduct = -det / (std::sqrt(x21 * x21 + y21 * y21) * std::sqrt(x43 * x43 + y43 * y43) + 1e-8);
            }
        }

        if (projection == Projections::spherical)
        {
            double x21 = GetDx(firstSegmentFistPoint, firstSegmentSecondPoint, projection);
            double y21 = GetDy(firstSegmentFistPoint, firstSegmentSecondPoint, projection);

            double x43 = GetDx(secondSegmentFistPoint, secondSegmentSecondPoint, projection);
            double y43 = GetDy(secondSegmentFistPoint, secondSegmentSecondPoint, projection);

            double x31 = GetDx(firstSegmentFistPoint, secondSegmentFistPoint, projection);
            double y31 = GetDy(firstSegmentFistPoint, secondSegmentFistPoint, projection);

            double det = x43 * y21 - y43 * x21;

            std::vector<double> values{ x21, y21, x43, y43 };
            double eps = std::max(0.00001 * (*std::max_element(values.begin(), values.end())), std::numeric_limits<double>::denorm_min());

            if (det < eps)
            {
                return isCrossing;
            }

            double sm = (y31 * y21 - x31 * y21) / det;
            double sl = (y31 * x43 - x31 * y43) / det;
            if (sm >= 0.0 && sm <= 1.0 && sl >= 0.0 && sl <= 1.0)
            {
                isCrossing = true;
            }
            intersection.x = firstSegmentFistPoint.x + sl * (firstSegmentSecondPoint.x - firstSegmentFistPoint.x);
            intersection.y = firstSegmentFistPoint.y + sl * (firstSegmentSecondPoint.x - firstSegmentFistPoint.y);
            crossProduct = -det;
            if (adimensional)
            {
                crossProduct = -det / (std::sqrt(x21 * x21 + y21 * y21) * std::sqrt(x43 * x43 + y43 * y43) + 1e-8);
            }
        }
        return isCrossing;
    }

    //faceAreaAndCenterOfMass: for cartesian, spherical point and spherical3dPoint
    static bool FaceAreaAndCenterOfMass(std::vector<Point>& polygon, int numberOfPolygonPoints, Projections projection, double& area, Point& centerOfMass)
    {
        if (numberOfPolygonPoints <= 0)
        {
            return false;
        }

        double minX = std::numeric_limits<double>::max();
        double minY = std::numeric_limits<double>::max();
        ReferencePoint(polygon, numberOfPolygonPoints, minX, minY, projection);

        Point reference{ minX, minY };
        area = 0.0;
        double xCenterOfMass = 0.0;
        double yCenterOfMass = 0.0;
        const double minArea = 1e-8;
        for (int p = 0; p < numberOfPolygonPoints; p++)
        {
            double dx0 = GetDx(reference, polygon[p], projection);
            double dy0 = GetDy(reference, polygon[p], projection);
            double dx1 = GetDx(reference, polygon[p + 1], projection);
            double dy1 = GetDy(reference, polygon[p + 1], projection);

            double xc = 0.5 * (dx0 + dx1);
            double yc = 0.5 * (dy0 + dy1);

            dx0 = GetDx(polygon[p], polygon[p + 1], projection);
            dy0 = GetDy(polygon[p], polygon[p + 1], projection);
            double dsx = dy0;
            double dsy = -dx0;
            double xds = xc * dsx + yc * dsy;
            area = area + 0.5 * xds;

            xCenterOfMass = xCenterOfMass + xds * xc;
            yCenterOfMass = yCenterOfMass + xds * yc;
        }
        area = std::abs(area) < minArea ? minArea : area;

        double fac = 1.0 / (3.0 * area);
        xCenterOfMass = fac * xCenterOfMass;
        yCenterOfMass = fac * yCenterOfMass;

        if (projection == Projections::spherical)
        {
            yCenterOfMass = yCenterOfMass / (earth_radius * degrad_hp);
            xCenterOfMass = xCenterOfMass / (earth_radius * degrad_hp * std::cos((yCenterOfMass + minY) * degrad_hp));
        }

        centerOfMass.x = xCenterOfMass + minX;
        centerOfMass.y = yCenterOfMass + minY;

        area = std::abs(area);

        return true;
    }

    static bool ComputePolygonCircumenter(std::vector<Point>& polygon,
        std::vector<Point>& middlePoints,
        std::vector<Point>& normals,
        int numNodes,
        const std::vector<int>& edgesNumFaces,
        Projections projection,
        const double weightCircumCenter,
        Point& result)
    {
        const int maximumNumberCircumcenterIterations = 100;

        Point centerOfMass;
        double area;
        FaceAreaAndCenterOfMass(polygon, numNodes, projection, area, centerOfMass);

        double xCenter = 0;
        double yCenter = 0;
        for (int n = 0; n < numNodes; n++)
        {
            xCenter += polygon[n].x;
            yCenter += polygon[n].y;
        }
        centerOfMass.x = xCenter / numNodes;
        centerOfMass.y = yCenter / numNodes;

        // for triangles, for now assume cartesian kernel
        result = centerOfMass;
        if (numNodes == 3)
        {
            CircumcenterOfTriangle(polygon[0], polygon[1], polygon[2], projection, result);
        }
        else if (!edgesNumFaces.empty())
        {
            Point estimatedCircumCenter = centerOfMass;

            int numValidEdges = 0;
            for (int n = 0; n < numNodes; ++n)
            {
                if (edgesNumFaces[n] == 2)
                {
                    numValidEdges++;
                }
            }

            if (numValidEdges > 0)
            {
                const double eps = 1e-3;
                for (int n = 0; n < numNodes; n++)
                {
                    int nextNode = n + 1;
                    if (nextNode == numNodes) nextNode = 0;
                    middlePoints[n].x = 0.5 * (polygon[n].x + polygon[nextNode].x);
                    middlePoints[n].y = 0.5 * (polygon[n].y + polygon[nextNode].y);
                    NormalVector(polygon[n], polygon[nextNode], middlePoints[n], normals[n], projection);
                }

                Point previousCircumCenter = estimatedCircumCenter;

                for (int iter = 0; iter < maximumNumberCircumcenterIterations; iter++)
                {
                    previousCircumCenter = estimatedCircumCenter;
                    for (int n = 0; n < numNodes; n++)
                    {
                        if (edgesNumFaces[n] == 2)
                        {
                            double dx = GetDx(middlePoints[n], estimatedCircumCenter, projection);
                            double dy = GetDy(middlePoints[n], estimatedCircumCenter, projection);
                            double increment = -0.1 * DotProduct(dx, dy, normals[n].x, normals[n].y);
                            Add(estimatedCircumCenter, normals[n], increment, projection);
                        }
                    }
                    if (iter > 0 &&
                        abs(estimatedCircumCenter.x - previousCircumCenter.x) < eps &&
                        abs(estimatedCircumCenter.y - previousCircumCenter.y) < eps)
                    {
                        result = estimatedCircumCenter;
                        break;
                    }
                }
            }
        }


        if (weightCircumCenter <= 1.0 && weightCircumCenter >= 0.0)
        {
            double localWeightCircumCenter = 1.0;
            if (numNodes > 3)
            {
                localWeightCircumCenter = weightCircumCenter;
            }

            for (int n = 0; n < numNodes; n++)
            {
                polygon[n].x = localWeightCircumCenter * polygon[n].x + (1.0 - localWeightCircumCenter) * centerOfMass.x;
                polygon[n].y = localWeightCircumCenter * polygon[n].y + (1.0 - localWeightCircumCenter) * centerOfMass.y;
            }
            polygon[numNodes] = polygon[0];

            const auto isCircumcenterInside = IsPointInPolygonNodes(result, polygon, 0, numNodes - 1);

            if (!isCircumcenterInside)
            {
                for (int n = 0; n < numNodes; n++)
                {
                    int nextNode = n + 1;
                    if (nextNode == numNodes) nextNode = 0;
                    Point intersection;
                    double crossProduct;
                    double firstRatio;
                    double secondRatio;
                    bool areLineCrossing = AreLinesCrossing(centerOfMass, result, polygon[n], polygon[nextNode], false, intersection, crossProduct, firstRatio, secondRatio, projection);
                    if (areLineCrossing)
                    {
                        result = intersection;
                        break;
                    }
                }
            }
        }

        return true;
    }

    static bool Averaging(const std::vector<Sample>& samples,
        int numPolygonNodes,
        const std::vector<Point>& polygon,
        const Point centerOfMass,
        const Projections& projection,
        SpatialTrees::RTree& rtree,
        int averagingMethod,
        double& result)
    {
        result = doubleMissingValue;
        std::vector<Point> searchPolygon(numPolygonNodes);

        // averaging settings
        const double relativeFaceSearchSize = 1.01;
        double minx = std::numeric_limits<double>::max();
        double maxx = std::numeric_limits<double>::min();
        double miny = std::numeric_limits<double>::max();
        double maxy = std::numeric_limits<double>::min();
        for (int i = 0; i < numPolygonNodes; i++)
        {
            searchPolygon[i] = polygon[i] * relativeFaceSearchSize + centerOfMass * (1 - relativeFaceSearchSize);
            minx = std::min(minx, searchPolygon[i].x);
            maxx = std::max(maxx, searchPolygon[i].x);
            miny = std::min(miny, searchPolygon[i].y);
            maxy = std::max(maxy, searchPolygon[i].y);
        }

        if (projection == Projections::spherical && maxx - minx > 180.0)
        {

            double xmean = 0.5 * (maxx + minx);
            minx = std::numeric_limits<double>::max();
            maxx = std::numeric_limits<double>::min();
            for (int i = 0; i < numPolygonNodes; i++)
            {
                if (searchPolygon[i].x < xmean)
                {
                    searchPolygon[i].x = searchPolygon[i].x + 360.0;
                    minx = std::min(minx, searchPolygon[i].x);
                    maxx = std::max(maxx, searchPolygon[i].x);
                }
            }
        }

        double searchRadius = std::numeric_limits<double>::min();
        for (int i = 0; i < numPolygonNodes; i++)
        {
            double distance = SquaredDistance(centerOfMass, searchPolygon[i], projection);
            searchRadius = std::max(searchRadius, distance);
        }
        if (searchRadius <= 0.0)
        {
            return true;
        }
        searchRadius = std::sqrt(searchRadius);

        rtree.NearestNeighbours(centerOfMass, searchRadius);
        if (rtree.GetQueryResultSize() == 0)
        {
            return true;
        }

        int numValidSamplesInPolygon = 0;
        double wall = 0;
        bool firstValidSampleFound = false;
        for (int i = 0; i < rtree.GetQueryResultSize(); i++)
        {
            //do stuff based on the averaging method
            auto sampleIndex = rtree.GetQuerySampleIndex(i);
            if (samples[sampleIndex].value == doubleMissingValue)
            {
                continue;
            }

            Point samplePoint{ samples[sampleIndex].x, samples[sampleIndex].y };
            // assume here polygon has a size equal to numPolygonNodes + 1
            bool isInPolygon = IsPointInPolygonNodes(samplePoint, polygon, 0, numPolygonNodes);
            if (isInPolygon)
            {
                if (averagingMethod == SimpleAveraging)
                {
                    result += samples[sampleIndex].value;
                    numValidSamplesInPolygon++;
                }
                if (averagingMethod == KdTree)
                {
                    if (!firstValidSampleFound)
                    {
                        firstValidSampleFound = true;
                        result = samples[sampleIndex].value;
                    }
                    result = std::min(std::abs(result), std::abs(samples[sampleIndex].value));
                }
                if (averagingMethod == InverseWeightDistance)
                {
                    double distance = std::max(0.01, Distance(centerOfMass, samplePoint, projection));
                    double weight = 1.0 / distance;
                    wall += weight;
                    numValidSamplesInPolygon++;
                    result += weight * samples[sampleIndex].value;
                }
            }
        }

        if (averagingMethod == SimpleAveraging && numValidSamplesInPolygon > 0)
        {
            result /= numValidSamplesInPolygon;
            return true;
        }

        if (averagingMethod == InverseWeightDistance && numValidSamplesInPolygon > 0)
        {
            result /= wall;
            return true;
        }


        return true;
    }

    static int NextCircularForwardIndex(int currentIndex, int size)
    {
        int index = currentIndex + 1;
        if (index >= size)
        {
            index = index - size;
        }
        return index;
    }

    static int NextCircularBackwardIndex(int currentIndex, int size)
    {
        int index = currentIndex - 1;
        if (index < 0)
        {
            index = index + size;
        }
        return index;
    }

    static bool ComputeMiddlePoint(const Point& firstPoint, const Point& secondPoint, const Projections& projection, Point& centre)
    {

        centre = (firstPoint + secondPoint) * 0.5;
        if (projection == Projections::spherical)
        {
            centre.y = (firstPoint.y + secondPoint.y) / 2.0;
            const auto isFirstNodeOnPole = IsPointOnPole(firstPoint);
            const auto isSecondNodeOnPole = IsPointOnPole(secondPoint);

            if (isFirstNodeOnPole && !isSecondNodeOnPole)
            {
                centre.x = secondPoint.x;
            }
            else if (!isFirstNodeOnPole && isSecondNodeOnPole)
            {
                centre.x = firstPoint.x;
            }
            else
            {
                const auto maxx = std::max(firstPoint.x, secondPoint.x);
                const auto minx = std::min(firstPoint.x, secondPoint.x);

                if (maxx - minx)
                {
                    centre.x = centre.x + 180.0;
                }
            }
        }

        return true;
    }

}