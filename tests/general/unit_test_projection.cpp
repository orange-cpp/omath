//
// Created by Vlad on 27.08.2024.
//
#include <complex>
#include <gtest/gtest.h>
import omath.trigonometry.angle;
import omath.source_engine.camera;

#include <print>

TEST(UnitTestProjection, Projection)
{
    constexpr auto fov = omath::Angle<float, 0.f, 180.f, omath::AngleFlags::Clamped>::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    const auto projected = cam.world_to_screen({1000.f, 0, 50.f});
    const auto result = cam.screen_to_world(projected.value());
    const auto result2 = cam.world_to_screen(result.value());

    EXPECT_EQ(static_cast<omath::Vector2<float>>(projected.value()),
              static_cast<omath::Vector2<float>>(result2.value()));
    EXPECT_NEAR(projected->x, 960.f, 0.001f);
    EXPECT_NEAR(projected->y, 504.f, 0.001f);
    EXPECT_NEAR(projected->z, 1.f, 0.001f);
}