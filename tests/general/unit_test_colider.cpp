//
// Created by Vlad on 11/9/2025.
//
#include <gtest/gtest.h>
#include <omath/collision/mesh_collider.hpp>



TEST(UnitTestColider, CheckToWorld)
{
    const std::vector<omath::Vector3<float>> mesh = {{1.f, 1.f, 1.f}, {-1.f, -1.f, -1.f}};

    const omath::collision::MeshCollider collider(mesh, {0.f, 2.f, 0.f});

    const auto vertex = collider.find_abs_furthest_vertex({1.f, 0.f, 0.f});

    EXPECT_EQ(vertex, omath::Vector3<float>(1.f, 3.f, 1.f));
}

TEST(UnitTestColider, FindFurthestVertex)
{
    const std::vector<omath::Vector3<float>> mesh = {{1.f, 1.f, 1.f}, {-1.f, -1.f, -1.f}};
    const omath::collision::MeshCollider collider(mesh, {0.f, 0.f, 0.f});
    const auto vertex = collider.find_furthest_vertex({1.f, 0.f, 0.f});
    EXPECT_EQ(vertex, omath::Vector3<float>(1.f, 1.f, 1.f));
}



