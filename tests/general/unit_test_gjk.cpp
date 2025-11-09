//
// Created by Vlad on 11/9/2025.
//
#include "omath/engines/source_engine/collider.hpp"
#include <gtest/gtest.h>
#include <omath/collision/gjk_algorithm.hpp>
#include <omath/engines/source_engine/mesh.hpp>
namespace
{
    const omath::source_engine::Mesh mesh = {
            {{-1.f, -1.f, -1.f},
             {-1.f, -1.f, 1.f},
             {-1.f, 1.f, -1.f},
             {-1.f, 1.f, 1.f},
             {1.f, 1.f, 1.f}, // x = +1 vertices (put {1,1,1} first in case your support breaks ties by first-hit)
             {1.f, 1.f, -1.f},
             {1.f, -1.f, 1.f},
             {1.f, -1.f, -1.f}},
            {}};
}
TEST(UnitTestGjk, TestCollisionTrue)
{

    const omath::source_engine::MeshCollider collider_a(mesh);

    auto mesh_b = mesh;
    mesh_b.set_origin({0.f, 0.5f, 0.f});
    const omath::source_engine::MeshCollider collider_b(mesh_b);

    const auto result =
            omath::collision::GjkAlgorithm<omath::source_engine::MeshCollider>::is_collide(collider_a, collider_b);

    EXPECT_TRUE(result);
}
TEST(UnitTestGjk, TestCollisionFalse)
{
    const omath::source_engine::MeshCollider collider_a(mesh);
    auto mesh_b = mesh;
    mesh_b.set_origin({0.f, 2.1f, 0.f});
    const omath::source_engine::MeshCollider collider_b(mesh_b);

    const auto result = omath::collision::GjkAlgorithm<omath::source_engine::MeshCollider>::is_collide(collider_a, collider_b);

    EXPECT_FALSE(result);
}

TEST(UnitTestGjk, TestCollisionEqualOrigin)
{
    const omath::source_engine::MeshCollider collider_a(mesh);
    const omath::source_engine::MeshCollider collider_b(mesh);

    const auto result = omath::collision::GjkAlgorithm<omath::source_engine::MeshCollider>::is_collide(collider_a, collider_b);

    EXPECT_TRUE(result);
}