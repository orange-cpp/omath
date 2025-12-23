#include <gtest/gtest.h>
#include "omath/pathfinding/navigation_mesh.hpp"

using namespace omath;
using namespace omath::pathfinding;

TEST(NavigationMeshTests, SerializeDeserializeRoundTrip)
{
    NavigationMesh nav;
    Vector3<float> a{0.f,0.f,0.f};
    Vector3<float> b{1.f,0.f,0.f};
    Vector3<float> c{0.f,1.f,0.f};

    nav.m_vertex_map.emplace(a, std::vector<Vector3<float>>{b,c});
    nav.m_vertex_map.emplace(b, std::vector<Vector3<float>>{a});
    nav.m_vertex_map.emplace(c, std::vector<Vector3<float>>{a});

    auto data = nav.serialize();
    NavigationMesh nav2;
    EXPECT_NO_THROW(nav2.deserialize(data));

    // verify neighbors preserved
    EXPECT_EQ(nav2.m_vertex_map.size(), nav.m_vertex_map.size());
    EXPECT_EQ(nav2.get_neighbors(a).size(), 2u);
}

TEST(NavigationMeshTests, GetClosestVertexWhenEmpty)
{
    const NavigationMesh nav;
    constexpr Vector3<float> p{5.f,5.f,5.f};
    const auto res = nav.get_closest_vertex(p);
    EXPECT_FALSE(res.has_value());
}
