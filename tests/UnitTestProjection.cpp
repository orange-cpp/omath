//
// Created by Vlad on 27.08.2024.
//
#include <complex>
#include <gtest/gtest.h>
#include <omath/Matrix.h>
#include <print>
#include <omath/projection/Camera.h>

TEST(UnitTestProjection, Projection)
{
    const omath::projection::Camera camera({0.f, 0.f, 0.f}, {0, 0.f, 0.f} , {1920.f, 1080.f}, 110.f, 0.375f, 5000.f);

    EXPECT_EQ(camera.WorldToScreen({100, 0, 0}).value(), omath::Vector2(960, 540));
    EXPECT_EQ(camera.WorldToScreen({49.23, 0, 0}).value(), omath::Vector2(960, 540));
    const auto proj = camera.WorldToScreen({100, 50, -69});
    std::print("{} {}", proj->x, proj->y);
    EXPECT_EQ(camera.WorldToScreen({100, 10, 0}).value(), omath::Vector2(909.58887, 540));
    EXPECT_EQ(camera.WorldToScreen({100, 10, 8}).value(), omath::Vector2(909.58887, 499.67108));
    EXPECT_EQ(camera.WorldToScreen({100, 50, -69}).value(), omath::Vector2(707.9442, 887.83704));

    EXPECT_FALSE(camera.WorldToScreen({-10,0, 0}).has_value());
}