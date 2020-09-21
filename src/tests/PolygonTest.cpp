#include "../Mesh.hpp"
#include "../Entities.hpp"
#include "../Polygons.hpp"
#include "../Splines.hpp"
#include <gtest/gtest.h>

TEST(Polygons, MeshBoundaryToPolygon)
{
    //One gets the edges
    std::vector<MeshKernel::Point> nodes;

    nodes.push_back({ 322.252624511719,454.880187988281 });
    nodes.push_back({ 227.002044677734,360.379241943359 });
    nodes.push_back({ 259.252227783203,241.878051757813 });
    nodes.push_back({ 428.003295898438,210.377746582031 });
    nodes.push_back({ 536.003967285156,310.878753662109 });
    nodes.push_back({ 503.753784179688,432.379974365234 });
    nodes.push_back({ 350.752807617188,458.630249023438 });
    nodes.push_back({ 343.15053976393,406.232256102912 });
    nodes.push_back({ 310.300984548069,319.41005739802 });
    nodes.push_back({ 423.569603308318,326.17986967523 });

    std::vector<MeshKernel::Edge> edges;
    // Local edges
    edges.push_back({ 3, 9 });
    edges.push_back({ 9, 2 });
    edges.push_back({ 2, 3 });
    edges.push_back({ 3, 4 });
    edges.push_back({ 4, 9 });
    edges.push_back({ 2, 8 });
    edges.push_back({ 8, 1 });
    edges.push_back({ 1, 2 });
    edges.push_back({ 9, 8 });
    edges.push_back({ 8, 7 });
    edges.push_back({ 7, 1 });
    edges.push_back({ 9, 10 });
    edges.push_back({ 10, 8 });
    edges.push_back({ 4, 5 });
    edges.push_back({ 5, 10 });
    edges.push_back({ 10, 4 });
    edges.push_back({ 8, 6 });
    edges.push_back({ 6, 7 });
    edges.push_back({ 10, 6 });
    edges.push_back({ 5, 6 });

    for (int i = 0; i < edges.size(); i++)
    {
        edges[i].first -= 1;
        edges[i].second -= 1;
    }

    // now build node-edge mapping
    auto mesh=std::make_shared<MeshKernel::Mesh>();
    mesh->Set(edges, nodes, MeshKernel::Projections::cartesian);

    auto polygons =std::make_shared<MeshKernel::Polygons>();
    const std::vector<MeshKernel::Point> polygon;
    std::vector<MeshKernel::Point> meshBoundaryPolygon;
    int numNodesBoundaryPolygons;
    polygons->Set(polygon, MeshKernel::Projections::cartesian);
    polygons->MeshBoundaryToPolygon(*mesh, 0, meshBoundaryPolygon, numNodesBoundaryPolygons);


    //constexpr double tolerance = 1e-2;

    //ASSERT_NEAR(325.590101919525, mesh.m_nodes[0].x, tolerance);
    //ASSERT_NEAR(229.213730481198, mesh.m_nodes[1].x, tolerance);
    //ASSERT_NEAR(263.439319753147, mesh.m_nodes[2].x, tolerance);
    //ASSERT_NEAR(429.191105834504, mesh.m_nodes[3].x, tolerance);
    //ASSERT_NEAR(535.865215426468, mesh.m_nodes[4].x, tolerance);
    //ASSERT_NEAR(503.753784179688, mesh.m_nodes[5].x, tolerance);
    //ASSERT_NEAR(354.048340705929, mesh.m_nodes[6].x, tolerance);
    //ASSERT_NEAR(346.790050854504, mesh.m_nodes[7].x, tolerance);
    //ASSERT_NEAR(315.030130405285, mesh.m_nodes[8].x, tolerance);
    //ASSERT_NEAR(424.314957449766, mesh.m_nodes[9].x, tolerance);

    //ASSERT_NEAR(455.319334078551, mesh.m_nodes[0].y, tolerance);
    //ASSERT_NEAR(362.573521507281, mesh.m_nodes[1].y, tolerance);
    //ASSERT_NEAR(241.096458631763, mesh.m_nodes[2].y, tolerance);
    //ASSERT_NEAR(211.483073921775, mesh.m_nodes[3].y, tolerance);
    //ASSERT_NEAR(311.401495506714, mesh.m_nodes[4].y, tolerance);
    //ASSERT_NEAR(432.379974365234, mesh.m_nodes[5].y, tolerance);
    //ASSERT_NEAR(458.064836627594, mesh.m_nodes[6].y, tolerance);
    //ASSERT_NEAR(405.311585650679, mesh.m_nodes[7].y, tolerance);
    //ASSERT_NEAR(319.612138503550, mesh.m_nodes[8].y, tolerance);
    //ASSERT_NEAR(327.102805172725, mesh.m_nodes[9].y, tolerance);
}

TEST(Polygons, CreatePointsInPolygons)
{
    // Prepare
    MeshKernel::Polygons  polygons;
    std::vector<MeshKernel::Point> nodes;

    nodes.push_back({ 302.002502,472.130371 });
    nodes.push_back({ 144.501526, 253.128174 });
    nodes.push_back({ 368.752930, 112.876755 });
    nodes.push_back({ 707.755005, 358.879242 });
    nodes.push_back({ 301.252502, 471.380371 });
    nodes.push_back({ 302.002502, 472.130371 });

    polygons.Set(nodes, MeshKernel::Projections::cartesian);

    // Execute
    std::vector<std::vector<MeshKernel::Point>> generatedPoints;
    bool successful = polygons.CreatePointsInPolygons(generatedPoints);
    ASSERT_TRUE(successful);

    // Assert
    const double tolerance = 1e-5;

    ASSERT_NEAR(302.00250199999999, generatedPoints[0][0].x, tolerance);
    ASSERT_NEAR(472.13037100000003, generatedPoints[0][0].y, tolerance);

    ASSERT_NEAR(144.50152600000001, generatedPoints[0][1].x, tolerance);
    ASSERT_NEAR(253.12817400000000, generatedPoints[0][1].y, tolerance);

    ASSERT_NEAR(368.75292999999999, generatedPoints[0][2].x, tolerance);
    ASSERT_NEAR(112.87675500000000, generatedPoints[0][2].y, tolerance);

    ASSERT_NEAR(707.75500499999998, generatedPoints[0][3].x, tolerance);
    ASSERT_NEAR(358.87924199999998, generatedPoints[0][3].y, tolerance);

    ASSERT_NEAR(301.25250199999999, generatedPoints[0][4].x, tolerance);
    ASSERT_NEAR(471.38037100000003, generatedPoints[0][4].y, tolerance);
}

TEST(Polygons, RefinePolygon)
{
    // Prepare
    MeshKernel::Polygons polygons;
    std::vector<MeshKernel::Point> nodes;

    nodes.push_back({ 0,0 });
    nodes.push_back({ 3, 0 });
    nodes.push_back({ 3, 3 });
    nodes.push_back({ 0, 3 });
    nodes.push_back({ 0, 0 });

    polygons.Set(nodes, MeshKernel::Projections::cartesian);

    // Execute
    std::vector<std::vector<MeshKernel::Point>> generatedPoints;
    std::vector<MeshKernel::Point> refinedPolygon;
    bool successful = polygons.RefinePolygonPart(0, 0, 1.0, refinedPolygon);
    ASSERT_TRUE(successful);

    ASSERT_EQ(13, refinedPolygon.size());
    const double tolerance = 1e-5;

    ASSERT_NEAR(0, refinedPolygon[0].x, tolerance);
    ASSERT_NEAR(1, refinedPolygon[1].x, tolerance);
    ASSERT_NEAR(2, refinedPolygon[2].x, tolerance);
    ASSERT_NEAR(3, refinedPolygon[3].x, tolerance);
    ASSERT_NEAR(3, refinedPolygon[4].x, tolerance);
    ASSERT_NEAR(3, refinedPolygon[5].x, tolerance);
    ASSERT_NEAR(3, refinedPolygon[6].x, tolerance);
    ASSERT_NEAR(2, refinedPolygon[7].x, tolerance);
    ASSERT_NEAR(1, refinedPolygon[8].x, tolerance);
    ASSERT_NEAR(0, refinedPolygon[9].x, tolerance);
    ASSERT_NEAR(0, refinedPolygon[10].x, tolerance);
    ASSERT_NEAR(0, refinedPolygon[11].x, tolerance);
    ASSERT_NEAR(0, refinedPolygon[12].x, tolerance);

    ASSERT_NEAR(0, refinedPolygon[0].y, tolerance);
    ASSERT_NEAR(0, refinedPolygon[1].y, tolerance);
    ASSERT_NEAR(0, refinedPolygon[2].y, tolerance);
    ASSERT_NEAR(0, refinedPolygon[3].y, tolerance);
    ASSERT_NEAR(1, refinedPolygon[4].y, tolerance);
    ASSERT_NEAR(2, refinedPolygon[5].y, tolerance);
    ASSERT_NEAR(3, refinedPolygon[6].y, tolerance);
    ASSERT_NEAR(3, refinedPolygon[7].y, tolerance);
    ASSERT_NEAR(3, refinedPolygon[8].y, tolerance);
    ASSERT_NEAR(3, refinedPolygon[9].y, tolerance);
    ASSERT_NEAR(2, refinedPolygon[10].y, tolerance);
    ASSERT_NEAR(1, refinedPolygon[11].y, tolerance);
    ASSERT_NEAR(0, refinedPolygon[12].y, tolerance);
}

TEST(Polygons, RefinePolygonOneSide)
{
    // Prepare
    MeshKernel::Polygons polygons;
    std::vector<MeshKernel::Point> nodes;

    nodes.push_back({ 0,0 });
    nodes.push_back({ 3, 0 });
    nodes.push_back({ 3, 3 });
    nodes.push_back({ 0, 3 });
    nodes.push_back({ 0, 0 });

    polygons.Set(nodes, MeshKernel::Projections::cartesian);

    // Execute
    std::vector<std::vector<MeshKernel::Point>> generatedPoints;
    std::vector<MeshKernel::Point> refinedPolygon;
    bool successful = polygons.RefinePolygonPart(0, 1, 1.0, refinedPolygon);
    ASSERT_TRUE(successful);

    ASSERT_EQ(7, refinedPolygon.size());
    const double tolerance = 1e-5;

    ASSERT_NEAR(0.0, refinedPolygon[0].x, tolerance);
    ASSERT_NEAR(1.0, refinedPolygon[1].x, tolerance);
    ASSERT_NEAR(2.0, refinedPolygon[2].x, tolerance);
    ASSERT_NEAR(3.0, refinedPolygon[3].x, tolerance);
    ASSERT_NEAR(3.0, refinedPolygon[4].x, tolerance);
    ASSERT_NEAR(0.0, refinedPolygon[5].x, tolerance);
    ASSERT_NEAR(0.0, refinedPolygon[6].x, tolerance);

    ASSERT_NEAR(0.0, refinedPolygon[0].y, tolerance);
    ASSERT_NEAR(0.0, refinedPolygon[1].y, tolerance);
    ASSERT_NEAR(0.0, refinedPolygon[2].y, tolerance);
    ASSERT_NEAR(0.0, refinedPolygon[3].y, tolerance);
    ASSERT_NEAR(3.0, refinedPolygon[4].y, tolerance);
    ASSERT_NEAR(3.0, refinedPolygon[5].y, tolerance);
    ASSERT_NEAR(0.0, refinedPolygon[6].y, tolerance);
}

TEST(Polygons, RefinePolygonLongerSquare)
{
    // Prepare
    MeshKernel::Polygons polygons;
    std::vector<MeshKernel::Point> nodes;

    nodes.push_back({ 0, 0 });
    nodes.push_back({ 3, 0 });
    nodes.push_back({ 3, 3 });
    nodes.push_back({ 0, 3.5 });
    nodes.push_back({ 0, 0 });

    polygons.Set(nodes, MeshKernel::Projections::cartesian);

    // Execute
    std::vector<std::vector<MeshKernel::Point>> generatedPoints;
    std::vector<MeshKernel::Point> refinedPolygon;
    bool successful = polygons.RefinePolygonPart(0, 0, 1.0, refinedPolygon);
    ASSERT_TRUE(successful);

    ASSERT_EQ(13, refinedPolygon.size());
    const double tolerance = 1e-5;

    ASSERT_NEAR(0, refinedPolygon[0].x, tolerance);
    ASSERT_NEAR(1, refinedPolygon[1].x, tolerance);
    ASSERT_NEAR(2, refinedPolygon[2].x, tolerance);
    ASSERT_NEAR(3, refinedPolygon[3].x, tolerance);
    ASSERT_NEAR(3, refinedPolygon[4].x, tolerance);
    ASSERT_NEAR(3, refinedPolygon[5].x, tolerance);
    ASSERT_NEAR(3, refinedPolygon[6].x, tolerance);
    ASSERT_NEAR(2.0136060761678563, refinedPolygon[7].x, tolerance);
    ASSERT_NEAR(1.0272121523357125, refinedPolygon[8].x, tolerance);
    ASSERT_NEAR(0.040818228503568754, refinedPolygon[9].x, tolerance);
    ASSERT_NEAR(0, refinedPolygon[10].x, tolerance);
    ASSERT_NEAR(0, refinedPolygon[11].x, tolerance);
    ASSERT_NEAR(0, refinedPolygon[12].x, tolerance);

    ASSERT_NEAR(0, refinedPolygon[0].y, tolerance);
    ASSERT_NEAR(0, refinedPolygon[1].y, tolerance);
    ASSERT_NEAR(0, refinedPolygon[2].y, tolerance);
    ASSERT_NEAR(0, refinedPolygon[3].y, tolerance);
    ASSERT_NEAR(1, refinedPolygon[4].y, tolerance);
    ASSERT_NEAR(2, refinedPolygon[5].y, tolerance);
    ASSERT_NEAR(3, refinedPolygon[6].y, tolerance);
    ASSERT_NEAR(3.1643989873053573, refinedPolygon[7].y, tolerance);
    ASSERT_NEAR(3.3287979746107146, refinedPolygon[8].y, tolerance);
    ASSERT_NEAR(3.4931969619160719, refinedPolygon[9].y, tolerance);
    ASSERT_NEAR(2.5413812651491092, refinedPolygon[10].y, tolerance);
    ASSERT_NEAR(1.5413812651491092, refinedPolygon[11].y, tolerance);
    ASSERT_NEAR(0.54138126514910923, refinedPolygon[12].y, tolerance);
}

TEST(Polygon, OffsetCopy)
{
    std::vector<MeshKernel::Point> nodes;
    nodes.push_back({ 296.752472, 397.879639 });
    nodes.push_back({ 294.502472, 256.128204 });
    nodes.push_back({ 578.754211, 244.128082 });
    nodes.push_back({ 587.754272, 400.129639 });
    nodes.push_back({ 308.002533, 397.879639 });
    nodes.push_back({ 296.752472, 397.879639 });

    MeshKernel::Polygons polygon;
    bool successful = polygon.Set(nodes, MeshKernel::Projections::cartesian);
    ASSERT_TRUE(successful);

    MeshKernel::Polygons newPolygon;
    double distance = 10.0;
    bool innerAndOuter = false;
    successful = polygon.OffsetCopy(0, distance, innerAndOuter, newPolygon);

    const double tolerance = 1e-5;

    ASSERT_NEAR(newPolygon.m_nodes[0].x, 286.75373149966771, tolerance);
    ASSERT_NEAR(newPolygon.m_nodes[1].x, 284.34914611880089, tolerance);
    ASSERT_NEAR(newPolygon.m_nodes[2].x, 588.17047010011993, tolerance);
    ASSERT_NEAR(newPolygon.m_nodes[3].x, 598.35275776004642, tolerance);
    ASSERT_NEAR(newPolygon.m_nodes[4].x, 307.96231942308754, tolerance);
    ASSERT_NEAR(newPolygon.m_nodes[5].x, 296.75247200000001, tolerance);

    ASSERT_NEAR(newPolygon.m_nodes[0].y, 398.03834755999270, tolerance);
    ASSERT_NEAR(newPolygon.m_nodes[1].y, 246.54793497426144, tolerance);
    ASSERT_NEAR(newPolygon.m_nodes[2].y, 233.72165300742589, tolerance);
    ASSERT_NEAR(newPolygon.m_nodes[3].y, 410.21520441451258, tolerance);
    ASSERT_NEAR(newPolygon.m_nodes[4].y, 407.87963900000000, tolerance);
    ASSERT_NEAR(newPolygon.m_nodes[5].y, 407.87963900000000, tolerance);
    
}