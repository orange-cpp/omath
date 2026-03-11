#include <gtest/gtest.h>
#include "omath/pathfinding/navigation_mesh.hpp"

using namespace omath;
using namespace omath::pathfinding;

TEST(NavigationMeshTests, SerializeDeserializeRoundTrip)
{
    NavigationMesh nav;
    Vector3<float> a{0.f, 0.f, 0.f};
    Vector3<float> b{1.f, 0.f, 0.f};
    Vector3<float> c{0.f, 1.f, 0.f};

    nav.m_vertex_map.emplace(a, std::vector<Vector3<float>>{b, c});
    nav.m_vertex_map.emplace(b, std::vector<Vector3<float>>{a});
    nav.m_vertex_map.emplace(c, std::vector<Vector3<float>>{a});

    std::string data = nav.serialize();
    NavigationMesh nav2;
    EXPECT_NO_THROW(nav2.deserialize(data));

    EXPECT_EQ(nav2.m_vertex_map.size(), nav.m_vertex_map.size());
    EXPECT_EQ(nav2.get_neighbors(a).size(), 2u);
}

TEST(NavigationMeshTests, GetClosestVertexWhenEmpty)
{
    const NavigationMesh nav;
    constexpr Vector3<float> p{5.f, 5.f, 5.f};
    const auto res = nav.get_closest_vertex(p);
    EXPECT_FALSE(res.has_value());
}

TEST(NavigationMeshTests, SerializeEmptyMesh)
{
    const NavigationMesh nav;
    const std::string data = nav.serialize();
    EXPECT_TRUE(data.empty());
}

TEST(NavigationMeshTests, DeserializeEmptyString)
{
    NavigationMesh nav;
    EXPECT_NO_THROW(nav.deserialize(""));
    EXPECT_TRUE(nav.empty());
}

TEST(NavigationMeshTests, SerializeProducesHumanReadableText)
{
    NavigationMesh nav;
    nav.m_vertex_map.emplace(Vector3<float>{1.f, 2.f, 3.f}, std::vector<Vector3<float>>{{4.f, 5.f, 6.f}});

    const std::string data = nav.serialize();

    // Must contain the vertex and neighbor coords as plain text
    EXPECT_NE(data.find("1"), std::string::npos);
    EXPECT_NE(data.find("2"), std::string::npos);
    EXPECT_NE(data.find("3"), std::string::npos);
    EXPECT_NE(data.find("4"), std::string::npos);
    EXPECT_NE(data.find("5"), std::string::npos);
    EXPECT_NE(data.find("6"), std::string::npos);
}

TEST(NavigationMeshTests, DeserializeRestoresNeighborValues)
{
    NavigationMesh nav;
    const Vector3<float> v{1.f, 2.f, 3.f};
    const Vector3<float> n1{4.f, 5.f, 6.f};
    const Vector3<float> n2{7.f, 8.f, 9.f};
    nav.m_vertex_map.emplace(v, std::vector<Vector3<float>>{n1, n2});

    NavigationMesh nav2;
    nav2.deserialize(nav.serialize());

    ASSERT_EQ(nav2.m_vertex_map.count(v), 1u);
    const auto& neighbors = nav2.get_neighbors(v);
    ASSERT_EQ(neighbors.size(), 2u);
    EXPECT_EQ(neighbors[0], n1);
    EXPECT_EQ(neighbors[1], n2);
}

TEST(NavigationMeshTests, DeserializeOverwritesPreviousData)
{
    NavigationMesh nav;
    const Vector3<float> v{1.f, 0.f, 0.f};
    nav.m_vertex_map.emplace(v, std::vector<Vector3<float>>{});

    // Load a different mesh into the same object
    NavigationMesh other;
    const Vector3<float> a{10.f, 20.f, 30.f};
    other.m_vertex_map.emplace(a, std::vector<Vector3<float>>{});

    nav.deserialize(other.serialize());

    EXPECT_EQ(nav.m_vertex_map.size(), 1u);
    EXPECT_EQ(nav.m_vertex_map.count(v), 0u);
    EXPECT_EQ(nav.m_vertex_map.count(a), 1u);
}

TEST(NavigationMeshTests, RoundTripNegativeAndFractionalCoords)
{
    NavigationMesh nav;
    const Vector3<float> v{-1.5f, 0.25f, -3.75f};
    const Vector3<float> n{100.f, -200.f, 0.001f};
    nav.m_vertex_map.emplace(v, std::vector<Vector3<float>>{n});

    NavigationMesh nav2;
    nav2.deserialize(nav.serialize());

    ASSERT_EQ(nav2.m_vertex_map.count(v), 1u);
    const auto& neighbors = nav2.get_neighbors(v);
    ASSERT_EQ(neighbors.size(), 1u);
    EXPECT_NEAR(neighbors[0].x, n.x, 1e-3f);
    EXPECT_NEAR(neighbors[0].y, n.y, 1e-3f);
    EXPECT_NEAR(neighbors[0].z, n.z, 1e-3f);
}

TEST(NavigationMeshTests, GetClosestVertexReturnsNearest)
{
    NavigationMesh nav;
    const Vector3<float> a{0.f, 0.f, 0.f};
    const Vector3<float> b{10.f, 0.f, 0.f};
    nav.m_vertex_map.emplace(a, std::vector<Vector3<float>>{});
    nav.m_vertex_map.emplace(b, std::vector<Vector3<float>>{});

    const auto res = nav.get_closest_vertex({1.f, 0.f, 0.f});
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), a);
}

TEST(NavigationMeshTests, VertexWithNoNeighborsRoundTrip)
{
    NavigationMesh nav;
    const Vector3<float> v{5.f, 5.f, 5.f};
    nav.m_vertex_map.emplace(v, std::vector<Vector3<float>>{});

    NavigationMesh nav2;
    nav2.deserialize(nav.serialize());

    ASSERT_EQ(nav2.m_vertex_map.count(v), 1u);
    EXPECT_TRUE(nav2.get_neighbors(v).empty());
}
