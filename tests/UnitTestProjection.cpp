//
// Created by Vlad on 27.08.2024.
//
#include <complex>
#include <gtest/gtest.h>
#include <omath/Matrix.hpp>
#include <print>
#include <omath/projection/Camera.hpp>

TEST(UnitTestProjection, Projection)
{
    const omath::projection::Camera camera({0.f, 0.f, 0.f}, {0, 0.f, 0.f} , {1920.f, 1080.f}, 110.f, 0.375f, 5000.f, 1.335f);

    const auto projected = camera.WorldToScreen({5000, 0, 0});


    EXPECT_TRUE(projected.has_value());
    EXPECT_EQ(projected->z, 1.f);
}