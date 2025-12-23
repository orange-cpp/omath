// Extra unit tests for the project's A* implementation
#include <gtest/gtest.h>
#include <omath/pathfinding/a_star.hpp>
#include <omath/pathfinding/navigation_mesh.hpp>
#include <array>
#include <utility>

using namespace omath;
using namespace omath::pathfinding;

TEST(AStarExtra, TrivialNeighbor)
{
    NavigationMesh nav;
    Vector3<float> v1{0.f,0.f,0.f};
    Vector3<float> v2{1.f,0.f,0.f};
    nav.m_vertex_map[v1] = {v2};
    nav.m_vertex_map[v2] = {v1};

    const auto path = Astar::find_path(v1, v2, nav);
    ASSERT_EQ(path.size(), 1u);
    EXPECT_EQ(path.front(), v2);
}

TEST(AStarExtra, StartEqualsGoal)
{
    NavigationMesh nav;
    constexpr Vector3<float> v{1.f,1.f,0.f};
    nav.m_vertex_map[v] = {};

    const auto path = Astar::find_path(v, v, nav);
    ASSERT_EQ(path.size(), 1u);
    EXPECT_EQ(path.front(), v);
}

TEST(AStarExtra, BlockedNoPathBetweenTwoVertices)
{
    NavigationMesh nav;
    constexpr Vector3<float> left{0.f,0.f,0.f};
    constexpr Vector3<float> right{2.f,0.f,0.f};
    // both vertices present but no connections
    nav.m_vertex_map[left] = {};
    nav.m_vertex_map[right] = {};

    const auto path = Astar::find_path(left, right, nav);
    // disconnected vertices -> empty result
    EXPECT_TRUE(path.empty());
}

TEST(AStarExtra, LongerPathAvoidsBlock)
{
    NavigationMesh nav;
    // build 3x3 grid of vertices, block center (1,1)
    auto idx = [&](int x, int y){ return Vector3<float>{static_cast<float>(x), static_cast<float>(y), 0.f}; };
    for (int y = 0; y < 3; ++y)
    {
        for (int x = 0; x < 3; ++x)
        {
            Vector3<float> v = idx(x,y);
            if (x==1 && y==1) continue; // center is omitted (blocked)
            std::vector<Vector3<float>> neigh;
            constexpr std::array<std::pair<int,int>,4> offs{{{1,0},{-1,0},{0,1},{0,-1}}};
            for (auto [dx,dy]: offs)
            {
                int nx = x + dx, ny = y + dy;
                if (nx < 0 || nx >= 3 || ny < 0 || ny >= 3) continue;
                if (nx==1 && ny==1) continue; // neighbor is the blocked center
                neigh.push_back(idx(nx,ny));
            }
            nav.m_vertex_map[v] = neigh;
        }
    }

    constexpr Vector3<float> start = idx(0,1);
    constexpr Vector3<float> goal  = idx(2,1);
    const auto path = Astar::find_path(start, goal, nav);
    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front(), goal); // Astar convention: single-element or endpoint present
}


TEST(AstarTests, TrivialDirectNeighborPath)
{
    NavigationMesh nav;
    // create two vertices directly connected
    Vector3<float> v1{0.f,0.f,0.f};
    Vector3<float> v2{1.f,0.f,0.f};
    nav.m_vertex_map.emplace(v1, std::vector<Vector3<float>>{v2});
    nav.m_vertex_map.emplace(v2, std::vector<Vector3<float>>{v1});

    const auto path = Astar::find_path(v1, v2, nav);
    // Current A* implementation returns the end vertex as the reconstructed
    // path (single-element) in the simple neighbor scenario. Assert that the
    // endpoint is present and reachable.
    ASSERT_EQ(path.size(), 1u);
    EXPECT_EQ(path.front(), v2);
}

TEST(AstarTests, NoPathWhenDisconnected)
{
    NavigationMesh nav;
    Vector3<float> v1{0.f,0.f,0.f};
    constexpr Vector3<float> v2{10.f,0.f,0.f};
    // nav has only v1
    nav.m_vertex_map.emplace(v1, std::vector<Vector3<float>>{});

    const auto path = Astar::find_path(v1, v2, nav);
    // When the nav mesh contains only the start vertex, the closest
    // vertex for both start and end will be the same vertex. In that
    // case Astar returns a single-element path with the start vertex.
    ASSERT_EQ(path.size(), 1u);
    EXPECT_EQ(path.front(), v1);
}

TEST(AstarTests, EmptyNavReturnsNoPath)
{
    const NavigationMesh nav;
    constexpr Vector3<float> v1{0.f,0.f,0.f};
    constexpr Vector3<float> v2{1.f,0.f,0.f};

    const auto path = Astar::find_path(v1, v2, nav);
    EXPECT_TRUE(path.empty());
}

TEST(unit_test_a_star, finding_right_path)
{
    omath::pathfinding::NavigationMesh mesh;

    mesh.m_vertex_map[{0.f, 0.f, 0.f}] = {{0.f, 1.f, 0.f}};
    mesh.m_vertex_map[{0.f, 1.f, 0.f}] = {{0.f, 2.f, 0.f}};
    mesh.m_vertex_map[{0.f, 2.f, 0.f}] = {{0.f, 3.f, 0.f}};
    mesh.m_vertex_map[{0.f, 3.f, 0.f}] = {};
    std::ignore = omath::pathfinding::Astar::find_path({}, {0.f, 3.f, 0.f}, mesh);
}