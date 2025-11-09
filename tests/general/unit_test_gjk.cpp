//
// Created by Vlad on 11/9/2025.
//
#include <gtest/gtest.h>
#include <omath/collision/gjk_algorithm.hpp>

TEST(UnitTestGjk, TestCollisionTrue)
{
    const std::vector<omath::Vector3<float>> mesh = {
        {-1.f, -1.f, -1.f},
        {-1.f, -1.f,  1.f},
        {-1.f,  1.f, -1.f},
        {-1.f,  1.f,  1.f},
        { 1.f,  1.f,  1.f}, // x = +1 vertices (put {1,1,1} first in case your support breaks ties by first-hit)
        { 1.f,  1.f, -1.f},
        { 1.f, -1.f,  1.f},
        { 1.f, -1.f, -1.f}
    };

    const omath::collision::MeshCollider collider_a(mesh, {0.f, 0.f, 0.f});
    const omath::collision::MeshCollider collider_b(mesh, {0.f, 0.5f, 0.f});

    const auto result = omath::collision::GjkAlgorithm<>::is_collide(collider_a, collider_b);

    EXPECT_TRUE(result);
}
TEST(UnitTestGjk, TestCollisionFalse)
{
    const std::vector<omath::Vector3<float>> mesh = {
        {-1.f, -1.f, -1.f},
        {-1.f, -1.f,  1.f},
        {-1.f,  1.f, -1.f},
        {-1.f,  1.f,  1.f},
        { 1.f,  1.f,  1.f}, // x = +1 vertices (put {1,1,1} first in case your support breaks ties by first-hit)
        { 1.f,  1.f, -1.f},
        { 1.f, -1.f,  1.f},
        { 1.f, -1.f, -1.f}
    };

    const omath::collision::MeshCollider collider_a(mesh, {0.f, 0.f, 0.f});
    const omath::collision::MeshCollider collider_b(mesh, {0.f, 2.1f, 0.f});

    const auto result = omath::collision::GjkAlgorithm<>::is_collide(collider_a, collider_b);

    EXPECT_FALSE(result);
}

TEST(UnitTestGjk, TestCollisionEqualOrigin)
{
    const std::vector<omath::Vector3<float>> mesh = {
        {-1.f, -1.f, -1.f},
        {-1.f, -1.f,  1.f},
        {-1.f,  1.f, -1.f},
        {-1.f,  1.f,  1.f},
        { 1.f,  1.f,  1.f}, // x = +1 vertices (put {1,1,1} first in case your support breaks ties by first-hit)
        { 1.f,  1.f, -1.f},
        { 1.f, -1.f,  1.f},
        { 1.f, -1.f, -1.f}
    };

    const omath::collision::MeshCollider collider_a(mesh, {0.f, 0.f, 0.f});
    const omath::collision::MeshCollider collider_b(mesh, {0.f, 0.f, 0.f});

    const auto result = omath::collision::GjkAlgorithm<>::is_collide(collider_a, collider_b);

    EXPECT_TRUE(result);
}