//
// Created by Vlad on 11/9/2025.
//
#include <gtest/gtest.h>
#include <omath/collision/gjk_algorithm.hpp>

TEST(UnitTestGjk, TestCollision)
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
    const omath::collision::MeshCollider collider_b(mesh, {0.f, 3.f, 0.f});

    const auto result = omath::collision::GjkAlgorithm::check_collision(collider_a, collider_b);
}