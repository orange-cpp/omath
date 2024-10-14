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
    const omath::projection::Camera camera({0.f, 0.f, 0.f}, {0, 0.f, 0.f} , {1920.f, 1080.f}, 110.f, 0.1f, 500.f);

    const auto proj = camera.WorldToScreen({500, 0, 0});

    std::print("{} {} {}", proj->x, proj->y, proj->z);
    EXPECT_TRUE(proj.has_value());
}