//
// Created by Vlad on 27.08.2024.
//
#include <complex>
#include <gtest/gtest.h>
#include <print>

import omath.source_engine.camera;
import omath.angle;
import omath.view_angles;

TEST(UnitTestProjection, Projection)
{
    constexpr auto fov = omath::Angle<float, 0.f, 180.f, omath::AngleFlags::Clamped>::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    const auto projected = cam.world_to_screen({1000, 0, 50});

    EXPECT_NEAR(projected->x, 960.f, 0.001f);
    EXPECT_NEAR(projected->y, 504.f, 0.001f);
    EXPECT_NEAR(projected->z, 1.f, 0.001f);
}