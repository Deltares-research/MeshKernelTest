#include <chrono>
#include <gtest/gtest.h>
#include <random>

#include <MeshKernel/Constants.hpp>
#include <MeshKernel/Entities.hpp>
#include <MeshKernel/Mesh.hpp>
#include <MeshKernel/Mesh2D.hpp>
#include <MeshKernel/Polygons.hpp>
#include <TestUtils/Definitions.hpp>
#include <TestUtils/MakeMeshes.hpp>

TEST(Mesh2D, OneQuadTestConstructor)
{
    // 1 Setup
    std::vector<meshkernel::Point> nodes;
    nodes.push_back({0.0, 0.0});
    nodes.push_back({0.0, 10.0});
    nodes.push_back({10.0, 0.0});
    nodes.push_back({10.0, 10.0});
    std::vector<meshkernel::Edge> edges;
    edges.push_back({0, 2});
    edges.push_back({1, 3});
    edges.push_back({0, 1});
    edges.push_back({2, 3});

    // 2 Execution
    const auto mesh = meshkernel::Mesh2D(edges, nodes, meshkernel::Projection::cartesian);

    // 3 Validation
    // expect nodesEdges to be sorted ccw
    ASSERT_EQ(0, mesh.m_nodesEdges[0][0]);
    ASSERT_EQ(2, mesh.m_nodesEdges[0][1]);

    ASSERT_EQ(1, mesh.m_nodesEdges[1][0]);
    ASSERT_EQ(2, mesh.m_nodesEdges[1][1]);

    ASSERT_EQ(0, mesh.m_nodesEdges[2][0]);
    ASSERT_EQ(3, mesh.m_nodesEdges[2][1]);

    ASSERT_EQ(1, mesh.m_nodesEdges[3][0]);
    ASSERT_EQ(3, mesh.m_nodesEdges[3][1]);

    // each node has two edges int this case
    ASSERT_EQ(2, mesh.m_nodesNumEdges[0]);
    ASSERT_EQ(2, mesh.m_nodesNumEdges[1]);
    ASSERT_EQ(2, mesh.m_nodesNumEdges[2]);
    ASSERT_EQ(2, mesh.m_nodesNumEdges[3]);

    // the nodes composing the face, in ccw order
    ASSERT_EQ(0, mesh.m_facesNodes[0][0]);
    ASSERT_EQ(2, mesh.m_facesNodes[0][1]);
    ASSERT_EQ(3, mesh.m_facesNodes[0][2]);
    ASSERT_EQ(1, mesh.m_facesNodes[0][3]);

    // the edges composing the face, in ccw order
    ASSERT_EQ(0, mesh.m_facesEdges[0][0]);
    ASSERT_EQ(3, mesh.m_facesEdges[0][1]);
    ASSERT_EQ(1, mesh.m_facesEdges[0][2]);
    ASSERT_EQ(2, mesh.m_facesEdges[0][3]);

    // the found circumcenter for the face
    ASSERT_DOUBLE_EQ(5.0, mesh.m_facesCircumcenters[0].x);
    ASSERT_DOUBLE_EQ(5.0, mesh.m_facesCircumcenters[0].y);

    // each edge has only one face in this case
    ASSERT_EQ(1, mesh.m_edgesNumFaces[0]);
    ASSERT_EQ(1, mesh.m_edgesNumFaces[1]);
    ASSERT_EQ(1, mesh.m_edgesNumFaces[2]);
    ASSERT_EQ(1, mesh.m_edgesNumFaces[3]);

    // each edge is a boundary edge, so the second entry of edgesFaces is an invalid index (meshkernel::sizetMissingValue)
    ASSERT_EQ(meshkernel::sizetMissingValue, mesh.m_edgesFaces[0][1]);
    ASSERT_EQ(meshkernel::sizetMissingValue, mesh.m_edgesFaces[1][1]);
    ASSERT_EQ(meshkernel::sizetMissingValue, mesh.m_edgesFaces[2][1]);
    ASSERT_EQ(meshkernel::sizetMissingValue, mesh.m_edgesFaces[3][1]);
}

TEST(Mesh2D, TriangulateSamplesWithSkinnyTriangle)
{
    // Prepare
    std::vector<meshkernel::Point> nodes;

    nodes.push_back({302.002502, 472.130371});
    nodes.push_back({144.501526, 253.128174});
    nodes.push_back({368.752930, 112.876755});
    nodes.push_back({707.755005, 358.879242});
    nodes.push_back({301.252502, 471.380371});
    nodes.push_back({302.002502, 472.130371});

    meshkernel::Polygons polygons(nodes, meshkernel::Projection::cartesian);

    // Execute
    const auto generatedPoints = polygons.ComputePointsInPolygons();

    meshkernel::Mesh2D mesh(generatedPoints[0], polygons, meshkernel::Projection::cartesian);

    // Assert
    ASSERT_EQ(6, mesh.GetNumEdges());

    ASSERT_EQ(4, mesh.m_edges[0].first);
    ASSERT_EQ(1, mesh.m_edges[0].second);

    ASSERT_EQ(1, mesh.m_edges[1].first);
    ASSERT_EQ(2, mesh.m_edges[1].second);

    ASSERT_EQ(2, mesh.m_edges[2].first);
    ASSERT_EQ(4, mesh.m_edges[2].second);

    ASSERT_EQ(0, mesh.m_edges[3].first);
    ASSERT_EQ(2, mesh.m_edges[3].second);

    ASSERT_EQ(2, mesh.m_edges[4].first);
    ASSERT_EQ(3, mesh.m_edges[4].second);

    ASSERT_EQ(3, mesh.m_edges[5].first);
    ASSERT_EQ(0, mesh.m_edges[5].second);
}

TEST(Mesh2D, TriangulateSamples)
{
    // Prepare
    std::vector<meshkernel::Point> nodes;

    nodes.push_back({498.503152894023, 1645.82297461613});
    nodes.push_back({-5.90937355559299, 814.854361678898});
    nodes.push_back({851.30035347439, 150.079471329115});
    nodes.push_back({1411.11078745316, 1182.22995897746});
    nodes.push_back({501.418832237663, 1642.90729527249});
    nodes.push_back({498.503152894023, 1645.82297461613});

    meshkernel::Polygons polygons(nodes, meshkernel::Projection::cartesian);

    // Execute
    const auto generatedPoints = polygons.ComputePointsInPolygons();

    meshkernel::Mesh2D mesh(generatedPoints[0], polygons, meshkernel::Projection::cartesian);
}

TEST(Mesh2D, TwoTrianglesDuplicatedEdges)
{
    // 1 Setup
    std::vector<meshkernel::Point> nodes;
    nodes.push_back({0.0, 0.0});
    nodes.push_back({5.0, -5.0});
    nodes.push_back({10.0, 0.0});
    nodes.push_back({5.0, 5.0});
    std::vector<meshkernel::Edge> edges;
    edges.push_back({0, 3});
    edges.push_back({0, 2});
    edges.push_back({2, 3});
    edges.push_back({0, 1});
    edges.push_back({2, 1});

    meshkernel::Mesh2D mesh;
    // 2 Execution
    mesh = meshkernel::Mesh2D(edges, nodes, meshkernel::Projection::cartesian);

    // 3 Validation
    ASSERT_EQ(2, mesh.GetNumFaces());
}

TEST(Mesh2D, MeshBoundaryToPolygon)
{
    // 1 Setup
    std::vector<meshkernel::Point> nodes;
    nodes.push_back({0.0, 0.0});
    nodes.push_back({5.0, -5.0});
    nodes.push_back({10.0, 0.0});
    nodes.push_back({5.0, 5.0});
    std::vector<meshkernel::Edge> edges;
    edges.push_back({0, 3});
    edges.push_back({0, 2});
    edges.push_back({2, 3});
    edges.push_back({0, 1});
    edges.push_back({2, 1});

    meshkernel::Mesh2D mesh;
    mesh = meshkernel::Mesh2D(edges, nodes, meshkernel::Projection::cartesian);

    std::vector<meshkernel::Point> polygonNodes;
    const auto meshBoundaryPolygon = mesh.MeshBoundaryToPolygon(polygonNodes);

    const double tolerance = 1e-5;
    ASSERT_NEAR(0.0, meshBoundaryPolygon[0].x, tolerance);
    ASSERT_NEAR(5.0, meshBoundaryPolygon[1].x, tolerance);
    ASSERT_NEAR(10.0, meshBoundaryPolygon[2].x, tolerance);
    ASSERT_NEAR(5.0, meshBoundaryPolygon[3].x, tolerance);
    ASSERT_NEAR(0.0, meshBoundaryPolygon[4].x, tolerance);

    ASSERT_NEAR(0.0, meshBoundaryPolygon[0].y, tolerance);
    ASSERT_NEAR(5.0, meshBoundaryPolygon[1].y, tolerance);
    ASSERT_NEAR(0.0, meshBoundaryPolygon[2].y, tolerance);
    ASSERT_NEAR(-5.0, meshBoundaryPolygon[3].y, tolerance);
    ASSERT_NEAR(0.0, meshBoundaryPolygon[4].y, tolerance);
}

TEST(Mesh2D, HangingEdge)
{
    // 1 Setup
    std::vector<meshkernel::Point> nodes;
    nodes.push_back({0.0, 0.0});
    nodes.push_back({5.0, 0.0});
    nodes.push_back({3.0, 2.0});
    nodes.push_back({3.0, 4.0});

    std::vector<meshkernel::Edge> edges;
    edges.push_back({0, 1});
    edges.push_back({1, 3});
    edges.push_back({3, 0});
    edges.push_back({2, 1});

    meshkernel::Mesh2D mesh;
    mesh = meshkernel::Mesh2D(edges, nodes, meshkernel::Projection::cartesian);

    ASSERT_EQ(1, mesh.GetNumFaces());
}

TEST(Mesh2D, NodeMerging)
{
    // 1. Setup
    const int n = 10; // x
    const int m = 10; // y

    std::vector<std::vector<int>> indicesValues(n, std::vector<int>(m));
    std::vector<meshkernel::Point> nodes(n * m);
    std::size_t nodeIndex = 0;
    for (auto j = 0; j < m; ++j)
    {
        for (auto i = 0; i < n; ++i)
        {
            indicesValues[i][j] = i + j * n;
            nodes[nodeIndex] = {(double)i, (double)j};
            nodeIndex++;
        }
    }

    std::vector<meshkernel::Edge> edges((n - 1) * m + (m - 1) * n);
    std::size_t edgeIndex = 0;
    for (auto j = 0; j < m; ++j)
    {
        for (auto i = 0; i < n - 1; ++i)
        {
            edges[edgeIndex] = {indicesValues[i][j], indicesValues[i + 1][j]};
            edgeIndex++;
        }
    }

    for (auto j = 0; j < m - 1; ++j)
    {
        for (auto i = 0; i < n; ++i)
        {
            edges[edgeIndex] = {indicesValues[i][j + 1], indicesValues[i][j]};
            edgeIndex++;
        }
    }

    meshkernel::Mesh2D mesh;
    mesh = meshkernel::Mesh2D(edges, nodes, meshkernel::Projection::cartesian);

    // Add overlapping nodes
    double generatingDistance = std::sqrt(std::pow(0.001 * 0.9, 2) / 2.0);
    std::uniform_real_distribution<double> x_distribution(0.0, generatingDistance);
    std::uniform_real_distribution<double> y_distribution(0.0, generatingDistance);
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());

    nodes.resize(mesh.GetNumNodes() * 2);
    edges.resize(mesh.GetNumEdges() + mesh.GetNumNodes() * 2);
    int originalNodeIndex = 0;
    for (auto j = 0; j < m; ++j)
    {
        for (auto i = 0; i < n; ++i)
        {
            nodes[nodeIndex] = {i + x_distribution(generator), j + y_distribution(generator)};

            // add artificial edges
            auto edge = mesh.m_edges[mesh.m_nodesEdges[originalNodeIndex][0]];
            auto otherNode = edge.first + edge.second - originalNodeIndex;

            edges[edgeIndex] = {nodeIndex, otherNode};
            edgeIndex++;
            edges[edgeIndex] = {nodeIndex, originalNodeIndex};
            edgeIndex++;

            nodeIndex++;
            originalNodeIndex++;
        }
    }

    nodes.resize(nodeIndex);
    edges.resize(edgeIndex);

    // re set with augmented nodes
    mesh = meshkernel::Mesh2D(edges, nodes, meshkernel::Projection::cartesian);

    // 2. Act
    meshkernel::Polygons polygon;
    mesh.MergeNodesInPolygon(polygon, 0.001);

    // 3. Assert
    ASSERT_EQ(mesh.GetNumNodes(), n * m);
    ASSERT_EQ(mesh.GetNumEdges(), (n - 1) * m + (m - 1) * n);
}

TEST(Mesh2D, MillionQuads)
{
    const int n = 4; // x
    const int m = 4; // y

    std::vector<std::vector<int>> indicesValues(n, std::vector<int>(m));
    std::vector<meshkernel::Point> nodes(n * m);
    std::size_t nodeIndex = 0;
    for (auto j = 0; j < m; ++j)
    {
        for (auto i = 0; i < n; ++i)
        {
            indicesValues[i][j] = i + j * n;
            nodes[nodeIndex] = {(double)i, (double)j};
            nodeIndex++;
        }
    }

    std::vector<meshkernel::Edge> edges((n - 1) * m + (m - 1) * n);
    std::size_t edgeIndex = 0;
    for (auto j = 0; j < m; ++j)
    {
        for (auto i = 0; i < n - 1; ++i)
        {
            edges[edgeIndex] = {indicesValues[i][j], indicesValues[i + 1][j]};
            edgeIndex++;
        }
    }

    for (auto j = 0; j < m - 1; ++j)
    {
        for (auto i = 0; i < n; ++i)
        {
            edges[edgeIndex] = {indicesValues[i][j + 1], indicesValues[i][j]};
            edgeIndex++;
        }
    }

    // now build node-edge mapping
    auto start(std::chrono::steady_clock::now());
    const auto mesh = meshkernel::Mesh2D(edges, nodes, meshkernel::Projection::cartesian);
    auto end(std::chrono::steady_clock::now());

    double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    // std::cout << "Elapsed time " << elapsedTime << " s " << std::endl;
    // std::cout << "Number of found cells " << mesh.GetNumFaces() << std::endl;

    EXPECT_LE(elapsedTime, 5.0);
}

TEST(Mesh2D, InsertNodeInMeshWithExistingNodesRtreeTriggersRTreeReBuild)
{
    // Setup
    auto mesh = MakeRectangularMeshForTesting(2, 2, 1.0, meshkernel::Projection::cartesian);
    mesh->BuildTree(meshkernel::MeshLocations::Nodes);

    // insert nodes modifies the number of nodes, m_nodesRTreeRequiresUpdate is set to true
    meshkernel::Point newPoint{10.0, 10.0};

    const auto newNodeIndex = mesh->InsertNode(newPoint);

    mesh->ConnectNodes(0, newNodeIndex);

    // when m_nodesRTreeRequiresUpdate = true m_nodesRTree is not empty the mesh.m_nodesRTree is re-build
    mesh->Administrate();

    ASSERT_EQ(5, mesh->m_nodesRTree.Size());

    // even if m_edgesRTreeRequiresUpdate = true, m_edgesRTree is initially empty, so it is assumed that is not needed for searches
    ASSERT_EQ(0, mesh->m_edgesRTree.Size());
}

TEST(Mesh2D, DeleteNodeInMeshWithExistingNodesRtreeTriggersRTreeReBuild)
{
    // Setup
    auto mesh = MakeRectangularMeshForTesting(2, 2, 1.0, meshkernel::Projection::cartesian);

    meshkernel::Point newPoint{10.0, 10.0};
    mesh->BuildTree(meshkernel::MeshLocations::Nodes);
    const auto newNodeIndex = mesh->InsertNode(newPoint);

    // delete nodes modifies the number of nodes, m_nodesRTreeRequiresUpdate is set to true
    mesh->DeleteNode(0);

    // when m_nodesRTreeRequiresUpdate = true and m_nodesRTree is not empty the mesh.m_nodesRTree is re-build
    mesh->Administrate();

    ASSERT_EQ(3, mesh->m_nodesRTree.Size());
}

TEST(Mesh2D, ConnectNodesInMeshWithExistingEdgesRtreeTriggersRTreeReBuild)
{
    // 1 Setup
    auto mesh = MakeRectangularMeshForTesting(2, 2, 1.0, meshkernel::Projection::cartesian);
    mesh->BuildTree(meshkernel::MeshLocations::Edges);

    meshkernel::Point newPoint{10.0, 10.0};

    const auto newNodeIndex = mesh->InsertNode(newPoint);

    // connect nodes modifies the number of edges, m_nodesRTreeRequiresUpdate is set to true
    mesh->ConnectNodes(0, newNodeIndex);

    // when m_nodesRTreeRequiresUpdate = true m_nodesRTree is not empty the mesh.m_nodesRTree is re-build
    mesh->Administrate();

    // even if m_nodesRTreeRequiresUpdate = true, m_nodesRTree is initially empty, so it is assumed that is not needed for searches
    ASSERT_EQ(0, mesh->m_nodesRTree.Size());

    ASSERT_EQ(5, mesh->m_edgesRTree.Size());
}

TEST(Mesh2D, DeleteEdgeInMeshWithExistingEdgesRtreeTriggersRTreeReBuild)
{
    // 1 Setup
    auto mesh = MakeRectangularMeshForTesting(2, 2, 1.0, meshkernel::Projection::cartesian);
    mesh->BuildTree(meshkernel::MeshLocations::Edges);

    // DeleteEdge modifies the number of edges, m_edgesRTreeRequiresUpdate is set to true
    mesh->DeleteEdge(0);

    // when m_edgesRTreeRequiresUpdate = true the mesh.m_edgesRTree is re-build with one less edge
    mesh->Administrate();

    ASSERT_EQ(3, mesh->m_edgesRTree.Size());
}

TEST(Mesh2D, GetNodeIndexShouldTriggerNodesRTreeBuild)
{
    // 1 Setup
    auto mesh = MakeRectangularMeshForTesting(2, 2, 1.0, meshkernel::Projection::cartesian);

    // By default, no nodesRTree is build
    ASSERT_EQ(0, mesh->m_nodesRTree.Size());

    // FindNodeCloseToAPoint builds m_nodesRTree for searching the nodes
    const auto nodeIndex = mesh->FindNodeCloseToAPoint({1.5, 1.5}, 10.0);

    // m_nodesRTree is build
    ASSERT_EQ(4, mesh->m_nodesRTree.Size());

    // m_edgesRTree is not build when searching for nodes
    ASSERT_EQ(0, mesh->m_edgesRTree.Size());
}

TEST(Mesh2D, FindEdgeCloseToAPointShouldTriggerEdgesRTreeBuild)
{
    // 1 Setup
    auto mesh = MakeRectangularMeshForTesting(2, 2, 1.0, meshkernel::Projection::cartesian);

    // FindEdgeCloseToAPoint builds m_edgesRTree for searching the edges
    const auto edgeIndex = mesh->FindEdgeCloseToAPoint({1.5, 1.5});

    // m_nodesRTree is not build when searching for edges
    ASSERT_EQ(0, mesh->m_nodesRTree.Size());

    // m_edgesRTree is build
    ASSERT_EQ(4, mesh->m_edgesRTree.Size());
}

TEST(Mesh2D, GetObtuseTriangles)
{
    // Setup a mesh with two triangles, one obtuse
    std::vector<meshkernel::Point> nodes{
        {0.0, 0.0},
        {3.0, 0.0},
        {-1.0, 2.0},
        {1.5, -2.0}};

    std::vector<meshkernel::Edge> edges{
        {0, 1},
        {1, 2},
        {2, 0},
        {0, 3},
        {3, 1}};

    meshkernel::Mesh2D mesh;
    mesh = meshkernel::Mesh2D(edges, nodes, meshkernel::Projection::cartesian);

    // execute, only one obtuse triangle should be found
    const auto obtuseTrianglesCount = mesh.GetObtuseTrianglesCenters().size();

    // assert a small flow edge is found
    ASSERT_EQ(1, obtuseTrianglesCount);
}

TEST(Mesh2D, GetSmallFlowEdgeCenters)
{
    // Setup a mesh with two triangles
    std::vector<meshkernel::Point> nodes{
        {0.0, 0.0},
        {1.0, 0.0},
        {1.0, 0.3},
        {1.0, -0.3}};

    std::vector<meshkernel::Edge> edges{
        {0, 3},
        {3, 1},
        {1, 0},
        {1, 2},
        {2, 0},
    };

    meshkernel::Mesh2D mesh = meshkernel::Mesh2D(edges, nodes, meshkernel::Projection::cartesian);

    // execute, by setting the smallFlowEdgesThreshold high, a small flow edge will be found
    const auto numSmallFlowEdgeFirstQuery = mesh.GetEdgesCrossingSmallFlowEdges(100).size();

    // execute, by setting the smallFlowEdgesThreshold low, no small flow edge will be found
    const auto numSmallFlowEdgeSecondQuery = mesh.GetEdgesCrossingSmallFlowEdges(0.0).size();

    // assert a small flow edge is found
    ASSERT_EQ(1, numSmallFlowEdgeFirstQuery);
    ASSERT_EQ(0, numSmallFlowEdgeSecondQuery);
}

TEST(Mesh2D, DeleteSmallFlowEdge)
{
    // Setup a mesh with eight triangles
    auto mesh = ReadLegacyMesh2DFromFile(TEST_FOLDER + "/data/RemoveSmallFlowEdgesTests/remove_small_flow_edges_net.nc");

    ASSERT_EQ(8, mesh->GetNumFaces());

    // After merging the number of faces is reduced
    mesh->DeleteSmallFlowEdges(1.0);

    ASSERT_EQ(3, mesh->GetNumFaces());
}

TEST(Mesh2D, DeleteSmallTrianglesAtBoundaries)
{
    // Setup a mesh with two triangles
    auto mesh = ReadLegacyMesh2DFromFile(TEST_FOLDER + "/data/RemoveSmallFlowEdgesTests/remove_small_flow_edges_quad_net.nc");

    ASSERT_EQ(2, mesh->GetNumFaces());

    // After merging
    mesh->DeleteSmallTrianglesAtBoundaries(0.6);

    ASSERT_EQ(1, mesh->GetNumFaces());

    const double tolerance = 1e-8;
    ASSERT_NEAR(364.17013549804688, mesh->m_nodes[0].x, tolerance);
    ASSERT_NEAR(295.21142578125000, mesh->m_nodes[1].x, tolerance);
    ASSERT_NEAR(421.46209716796875, mesh->m_nodes[2].x, tolerance);
    ASSERT_NEAR(359.79510498046875, mesh->m_nodes[3].x, tolerance);

    ASSERT_NEAR(374.00662231445313, mesh->m_nodes[0].y, tolerance);
    ASSERT_NEAR(300.48181152343750, mesh->m_nodes[1].y, tolerance);
    ASSERT_NEAR(295.33038330078125, mesh->m_nodes[2].y, tolerance);
    ASSERT_NEAR(398.59295654296875, mesh->m_nodes[3].y, tolerance);
}

TEST(Mesh2D, DeleteHangingEdge)
{
    // 1 Setup
    std::vector<meshkernel::Point> nodes;
    nodes.push_back({0.0, 0.0});
    nodes.push_back({5.0, 0.0});
    nodes.push_back({3.0, 2.0});
    nodes.push_back({3.0, 4.0});

    std::vector<meshkernel::Edge> edges;
    edges.push_back({0, 1});
    edges.push_back({1, 3});
    edges.push_back({3, 0});
    edges.push_back({2, 1});

    // Execute
    meshkernel::Mesh2D mesh;
    mesh = meshkernel::Mesh2D(edges, nodes, meshkernel::Projection::cartesian);

    // Assert
    ASSERT_EQ(1, mesh.GetNumFaces());
    ASSERT_EQ(4, mesh.GetNumEdges());

    // Execute
    auto hangingEdges = mesh.GetHangingEdges();

    // Assert
    ASSERT_EQ(1, hangingEdges.size());

    // Execute
    mesh.DeleteHangingEdges();
    hangingEdges = mesh.GetHangingEdges();

    // Assert
    ASSERT_EQ(0, hangingEdges.size());
}

TEST(Mesh2D, GetIntersectedEdgesFromPolyline)
{
    // 1. Setup
    auto mesh = MakeRectangularMeshForTesting(4, 4, 1.0, meshkernel::Projection::cartesian);

    std::vector<meshkernel::Point> boundaryLines;
    boundaryLines.emplace_back(0.5, 0.5);
    boundaryLines.emplace_back(2.5, 0.5);
    boundaryLines.emplace_back(2.5, 2.5);
    boundaryLines.emplace_back(0.5, 2.5);
    boundaryLines.emplace_back(0.5, 0.5);

    // 2. Execute
    const auto& [nodesOfIntersectedEdges, edgeAdimensionalIntersections, polyLineIndexes, lineAdimensionalIntersections] = mesh->GetIntersectedEdgesFromPolyline(boundaryLines);

    // 3. Assert
    ASSERT_EQ(nodesOfIntersectedEdges[0], 4);
    ASSERT_EQ(nodesOfIntersectedEdges[1], 5);
    ASSERT_EQ(nodesOfIntersectedEdges[2], 8);
    ASSERT_EQ(nodesOfIntersectedEdges[3], 9);
    ASSERT_EQ(nodesOfIntersectedEdges[4], 13);
    ASSERT_EQ(nodesOfIntersectedEdges[5], 9);
    ASSERT_EQ(nodesOfIntersectedEdges[6], 14);
    ASSERT_EQ(nodesOfIntersectedEdges[7], 10);
    ASSERT_EQ(nodesOfIntersectedEdges[8], 7);
    ASSERT_EQ(nodesOfIntersectedEdges[9], 6);
    ASSERT_EQ(nodesOfIntersectedEdges[10], 11);
    ASSERT_EQ(nodesOfIntersectedEdges[11], 10);
    ASSERT_EQ(nodesOfIntersectedEdges[12], 1);
    ASSERT_EQ(nodesOfIntersectedEdges[13], 5);
    ASSERT_EQ(nodesOfIntersectedEdges[14], 2);
    ASSERT_EQ(nodesOfIntersectedEdges[15], 6);

    ASSERT_EQ(edgeAdimensionalIntersections[0], 0.5);
    ASSERT_EQ(edgeAdimensionalIntersections[1], 0.5);
    ASSERT_EQ(edgeAdimensionalIntersections[2], 0.5);
    ASSERT_EQ(edgeAdimensionalIntersections[3], 0.5);
    ASSERT_EQ(edgeAdimensionalIntersections[4], 0.5);
    ASSERT_EQ(edgeAdimensionalIntersections[5], 0.5);
    ASSERT_EQ(edgeAdimensionalIntersections[6], 0.5);
    ASSERT_EQ(edgeAdimensionalIntersections[7], 0.5);

    ASSERT_EQ(polyLineIndexes[0], 0);
    ASSERT_EQ(polyLineIndexes[1], 0);
    ASSERT_EQ(polyLineIndexes[2], 1);
    ASSERT_EQ(polyLineIndexes[3], 1);
    ASSERT_EQ(polyLineIndexes[4], 2);
    ASSERT_EQ(polyLineIndexes[5], 2);
    ASSERT_EQ(polyLineIndexes[6], 3);
    ASSERT_EQ(polyLineIndexes[7], 3);

    ASSERT_EQ(lineAdimensionalIntersections[0], 0.25);
    ASSERT_EQ(lineAdimensionalIntersections[1], 0.75);
    ASSERT_EQ(lineAdimensionalIntersections[2], 0.25);
    ASSERT_EQ(lineAdimensionalIntersections[3], 0.75);
    ASSERT_EQ(lineAdimensionalIntersections[4], 0.75);
    ASSERT_EQ(lineAdimensionalIntersections[5], 0.25);
    ASSERT_EQ(lineAdimensionalIntersections[6], 0.75);
    ASSERT_EQ(lineAdimensionalIntersections[7], 0.25);
}

TEST(Mesh2D, GetIntersectedEdgesFromObliquePolyline)
{
    // 1. Setup
    auto mesh = MakeRectangularMeshForTesting(6, 6, 1.0, meshkernel::Projection::cartesian);

    std::vector<meshkernel::Point> boundaryLines;
    boundaryLines.emplace_back(3.9, 0.0);
    boundaryLines.emplace_back(0.0, 3.9);

    // 2. Execute
    const auto& [nodesOfIntersectedEdges, edgeAdimensionalIntersections, polyLineIndexes, lineAdimensionalIntersections] = mesh->GetIntersectedEdgesFromPolyline(boundaryLines);

    // 3. Assert
    ASSERT_EQ(nodesOfIntersectedEdges[0],  9 );
    ASSERT_EQ(nodesOfIntersectedEdges[1],  3 );
    ASSERT_EQ(nodesOfIntersectedEdges[2],  14);
    ASSERT_EQ(nodesOfIntersectedEdges[3],  8 );
    ASSERT_EQ(nodesOfIntersectedEdges[4],  19);
    ASSERT_EQ(nodesOfIntersectedEdges[5],  13);
    ASSERT_EQ(nodesOfIntersectedEdges[6],  24);
    ASSERT_EQ(nodesOfIntersectedEdges[7],  18);
    ASSERT_EQ(nodesOfIntersectedEdges[8],  4 );
    ASSERT_EQ(nodesOfIntersectedEdges[9],  3 );
    ASSERT_EQ(nodesOfIntersectedEdges[10], 9 );
    ASSERT_EQ(nodesOfIntersectedEdges[11], 8 );
    ASSERT_EQ(nodesOfIntersectedEdges[12], 14);
    ASSERT_EQ(nodesOfIntersectedEdges[13], 13);
    ASSERT_EQ(nodesOfIntersectedEdges[14], 19);
    ASSERT_EQ(nodesOfIntersectedEdges[15], 18);

    ASSERT_NEAR(edgeAdimensionalIntersections[0], 0.89999999999999991, 1e-8);
    ASSERT_NEAR(edgeAdimensionalIntersections[1], 0.89999999999999969, 1e-8);
    ASSERT_NEAR(edgeAdimensionalIntersections[2], 0.89999999999999980, 1e-8);
    ASSERT_NEAR(edgeAdimensionalIntersections[3], 0.89999999999999991, 1e-8);
    ASSERT_NEAR(edgeAdimensionalIntersections[4], 0.10000000000000014, 1e-8);
    ASSERT_NEAR(edgeAdimensionalIntersections[5], 0.10000000000000014, 1e-8);
    ASSERT_NEAR(edgeAdimensionalIntersections[6], 0.10000000000000014, 1e-8);
    ASSERT_NEAR(edgeAdimensionalIntersections[7], 0.10000000000000003, 1e-8);

    ASSERT_EQ(polyLineIndexes[0], 0);
    ASSERT_EQ(polyLineIndexes[1], 0);
    ASSERT_EQ(polyLineIndexes[2], 0);
    ASSERT_EQ(polyLineIndexes[3], 0);
    ASSERT_EQ(polyLineIndexes[4], 0);
    ASSERT_EQ(polyLineIndexes[5], 0);
    ASSERT_EQ(polyLineIndexes[6], 0);
    ASSERT_EQ(polyLineIndexes[7], 0);

    ASSERT_NEAR(lineAdimensionalIntersections[0], 0.76923076923076927, 1e-8);
    ASSERT_NEAR(lineAdimensionalIntersections[1], 0.51282051282051289, 1e-8);
    ASSERT_NEAR(lineAdimensionalIntersections[2], 0.25641025641025644, 1e-8);
    ASSERT_NEAR(lineAdimensionalIntersections[3], 0.00000000000000001, 1e-8);
    ASSERT_NEAR(lineAdimensionalIntersections[4], 1.00000000000000001, 1e-8);
    ASSERT_NEAR(lineAdimensionalIntersections[5], 0.74358974358974361, 1e-8);
    ASSERT_NEAR(lineAdimensionalIntersections[6], 0.48717948717948717, 1e-8);
    ASSERT_NEAR(lineAdimensionalIntersections[7], 0.23076923076923075, 1e-8);
}
