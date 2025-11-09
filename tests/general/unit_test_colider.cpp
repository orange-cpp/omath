//
// Created by Vlad on 11/9/2025.
//
#include "omath/engines/source_engine/collider.hpp"
#include <gtest/gtest.h>
#include <omath/collision/mesh_collider.hpp>

TEST(UnitTestColider, CheckToWorld)
{
    omath::source_engine::Mesh mesh = {std::vector<omath::Vector3<float>>{{1.f, 1.f, 1.f}, {-1.f, -1.f, -1.f}}, {}};
    mesh.set_origin({0, 2, 0});
    const omath::source_engine::MeshCollider collider(mesh);

    const auto vertex = collider.find_abs_furthest_vertex({1.f, 0.f, 0.f});

    EXPECT_EQ(vertex, omath::Vector3<float>(1.f, 3.f, 1.f));
}

TEST(UnitTestColider, FindFurthestVertex)
{
    const omath::source_engine::Mesh mesh = {{{1.f, 1.f, 1.f}, {-1.f, -1.f, -1.f}}, {}};
    const omath::source_engine::MeshCollider collider(mesh);
    const auto vertex = collider.find_furthest_vertex({1.f, 0.f, 0.f});
    EXPECT_EQ(vertex, omath::Vector3<float>(1.f, 1.f, 1.f));
}



