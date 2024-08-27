//
// Created by Vlad on 27.08.2024.
//
#include <gtest/gtest.h>
#include <omath/Matrix.h>
#include <print>
#include <omath/projection/Camera.h>

TEST(UnitTestProjection, IsPointOnScreen)
{
    const omath::projection::Camera camera({}, {90, 0.f, 0.f} , {1920.f, 1080.f, 0.f}, 120, 10, 100);

    const auto proj = camera.WorldToScreen({0,0,15});
    EXPECT_TRUE(proj.has_value());
}