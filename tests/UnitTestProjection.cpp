//
// Created by Vlad on 27.08.2024.
//
#include <complex>
#include <gtest/gtest.h>
#include <omath/Matrix.h>
#include <print>
#include <omath/projection/Camera.h>

TEST(UnitTestProjection, IsPointOnScreen)
{
    const omath::projection::Camera camera({5, 0, 0}, {0, 0.f, 0.f} , {1920.f, 1080.f, 0.f}, 110, 0.1, 500);

    const auto proj = camera.WorldToScreen({10, 0, 0});
    if (proj)
        std::print("{} {} {}", proj->x, proj->y, proj->z);
    EXPECT_TRUE(proj.has_value());
}