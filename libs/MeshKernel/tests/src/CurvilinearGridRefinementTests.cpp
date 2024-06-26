#include <gtest/gtest.h>

#include <MeshKernel/CurvilinearGrid/CurvilinearGrid.hpp>
#include <MeshKernel/CurvilinearGrid/CurvilinearGridRefinement.hpp>
#include <MeshKernel/Entities.hpp>

using namespace meshkernel;

TEST(CurvilinearGridRefinement, Compute_OnCurvilinearGrid_ShouldRefine)
{
    // Set-up
    lin_alg::Matrix<Point> grid(4, 4);

    grid << Point{0, 0}, Point{0, 10}, Point{0, 20}, Point{0, 30},
        Point{10, 0}, Point{10, 10}, Point{10, 20}, Point{10, 30},
        Point{20, 0}, Point{20, 10}, Point{20, 20}, Point{20, 30},
        Point{30, 0}, Point{30, 10}, Point{30, 20}, Point{30, 30};

    CurvilinearGrid curvilinearGrid(grid, Projection::cartesian);
    CurvilinearGridRefinement curvilinearGridRefinement(curvilinearGrid, 10);
    curvilinearGridRefinement.SetBlock({10, 20}, {20, 20});

    // Execute
    [[maybe_unused]] auto dummyUndoAction = curvilinearGridRefinement.Compute();

    // Assert
    ASSERT_EQ(4, curvilinearGrid.NumM());
    ASSERT_EQ(13, curvilinearGrid.NumN());

    constexpr double tolerance = 1e-12;
    ASSERT_NEAR(0.0, curvilinearGrid.GetNode(0, 0).x, tolerance);
    ASSERT_NEAR(10.0, curvilinearGrid.GetNode(1, 0).x, tolerance);

    ASSERT_NEAR(11.0, curvilinearGrid.GetNode(2, 0).x, tolerance);
    ASSERT_NEAR(12.0, curvilinearGrid.GetNode(3, 0).x, tolerance);
    ASSERT_NEAR(13.0, curvilinearGrid.GetNode(4, 0).x, tolerance);
    ASSERT_NEAR(14.0, curvilinearGrid.GetNode(5, 0).x, tolerance);
    ASSERT_NEAR(15.0, curvilinearGrid.GetNode(6, 0).x, tolerance);
    ASSERT_NEAR(16.0, curvilinearGrid.GetNode(7, 0).x, tolerance);
    ASSERT_NEAR(17.0, curvilinearGrid.GetNode(8, 0).x, tolerance);
    ASSERT_NEAR(18.0, curvilinearGrid.GetNode(9, 0).x, tolerance);
    ASSERT_NEAR(19.0, curvilinearGrid.GetNode(10, 0).x, tolerance);

    ASSERT_NEAR(20.0, curvilinearGrid.GetNode(11, 0).x, tolerance);
    ASSERT_NEAR(30.0, curvilinearGrid.GetNode(12, 0).x, tolerance);

    ASSERT_NEAR(0.0, curvilinearGrid.GetNode(0, 0).y, tolerance);
    ASSERT_NEAR(10.0, curvilinearGrid.GetNode(0, 1).y, tolerance);
    ASSERT_NEAR(20.0, curvilinearGrid.GetNode(0, 2).y, tolerance);
    ASSERT_NEAR(30.0, curvilinearGrid.GetNode(0, 3).y, tolerance);
}

TEST(CurvilinearGridRefinement, Compute_OnCurvilinearGridWithMissingFaces_ShouldRefine)
{
    // Set-up
    lin_alg::Matrix<Point> grid(6, 4);

    grid << Point{0, 0}, Point{0, 10}, Point{0, 20}, Point{0, 30},
        Point{10, 0}, Point{10, 10}, Point{10, 20}, Point{10, 30},
        Point{}, Point{}, Point{20, 20}, Point{20, 30},
        Point{}, Point{}, Point{30, 20}, Point{30, 30},
        Point{40, 0}, Point{40, 10}, Point{40, 20}, Point{40, 30},
        Point{50, 0}, Point{50, 10}, Point{50, 20}, Point{50, 30};

    CurvilinearGrid curvilinearGrid(grid, Projection::cartesian);
    CurvilinearGridRefinement curvilinearGridRefinement(curvilinearGrid, 10);
    curvilinearGridRefinement.SetBlock({10, 20}, {20, 20});

    // Execute
    [[maybe_unused]] auto dummyUndoAction = curvilinearGridRefinement.Compute();

    // Assert
    ASSERT_EQ(4, curvilinearGrid.NumM());
    ASSERT_EQ(15, curvilinearGrid.NumN());

    constexpr double tolerance = 1e-12;

    // vertical gridline 0
    ASSERT_NEAR(0.0, curvilinearGrid.GetNode(0, 0).x, tolerance);
    ASSERT_NEAR(0.0, curvilinearGrid.GetNode(0, 1).x, tolerance);
    ASSERT_NEAR(0.0, curvilinearGrid.GetNode(0, 2).x, tolerance);
    ASSERT_NEAR(0.0, curvilinearGrid.GetNode(0, 3).x, tolerance);

    ASSERT_NEAR(0.0, curvilinearGrid.GetNode(0, 0).y, tolerance);
    ASSERT_NEAR(10.0, curvilinearGrid.GetNode(0, 1).y, tolerance);
    ASSERT_NEAR(20.0, curvilinearGrid.GetNode(0, 2).y, tolerance);
    ASSERT_NEAR(30.0, curvilinearGrid.GetNode(0, 3).y, tolerance);

    // vertical gridline 2
    ASSERT_NEAR(constants::missing::doubleValue, curvilinearGrid.GetNode(2, 0).x, tolerance);
    ASSERT_NEAR(constants::missing::doubleValue, curvilinearGrid.GetNode(2, 1).x, tolerance);
    ASSERT_NEAR(11.0, curvilinearGrid.GetNode(2, 2).x, tolerance);
    ASSERT_NEAR(11.0, curvilinearGrid.GetNode(2, 3).x, tolerance);

    ASSERT_NEAR(constants::missing::doubleValue, curvilinearGrid.GetNode(3, 0).y, tolerance);
    ASSERT_NEAR(constants::missing::doubleValue, curvilinearGrid.GetNode(3, 1).y, tolerance);
    ASSERT_NEAR(20.0, curvilinearGrid.GetNode(3, 2).y, tolerance);
    ASSERT_NEAR(30.0, curvilinearGrid.GetNode(3, 3).y, tolerance);

    // vertical gridline 10
    ASSERT_NEAR(constants::missing::doubleValue, curvilinearGrid.GetNode(10, 0).x, tolerance);
    ASSERT_NEAR(constants::missing::doubleValue, curvilinearGrid.GetNode(10, 1).x, tolerance);
    ASSERT_NEAR(19.0, curvilinearGrid.GetNode(10, 2).x, tolerance);
    ASSERT_NEAR(19.0, curvilinearGrid.GetNode(10, 3).x, tolerance);

    ASSERT_NEAR(constants::missing::doubleValue, curvilinearGrid.GetNode(10, 0).y, tolerance);
    ASSERT_NEAR(constants::missing::doubleValue, curvilinearGrid.GetNode(10, 1).y, tolerance);
    ASSERT_NEAR(20.0, curvilinearGrid.GetNode(10, 2).y, tolerance);
    ASSERT_NEAR(30.0, curvilinearGrid.GetNode(10, 3).y, tolerance);

    // vertical gridline 11
    ASSERT_NEAR(constants::missing::doubleValue, curvilinearGrid.GetNode(11, 0).x, tolerance);
    ASSERT_NEAR(constants::missing::doubleValue, curvilinearGrid.GetNode(11, 1).x, tolerance);
    ASSERT_NEAR(20.0, curvilinearGrid.GetNode(11, 2).x, tolerance);
    ASSERT_NEAR(20.0, curvilinearGrid.GetNode(11, 3).x, tolerance);

    ASSERT_NEAR(constants::missing::doubleValue, curvilinearGrid.GetNode(11, 0).y, tolerance);
    ASSERT_NEAR(constants::missing::doubleValue, curvilinearGrid.GetNode(11, 1).y, tolerance);
    ASSERT_NEAR(20.0, curvilinearGrid.GetNode(11, 2).y, tolerance);
    ASSERT_NEAR(30.0, curvilinearGrid.GetNode(11, 3).y, tolerance);
}
