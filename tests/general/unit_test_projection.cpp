//
// Created by Vlad on 27.08.2024.
//
#include "omath/engines/unity_engine/camera.hpp"
#include <complex>
#include <gtest/gtest.h>
#include <omath/engines/source_engine/camera.hpp>
#include <omath/projection/camera.hpp>
#include <print>
#include <random>

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
TEST(UnitTestProjection, ScreenToNdcTopLeft)
{
    constexpr auto fov = omath::Angle<float, 0.f, 180.f, omath::AngleFlags::Clamped>::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);
    using ScreenStart = omath::source_engine::Camera::ScreenStart;

    const auto ndc_top_left = cam.screen_to_ndc<ScreenStart::TOP_LEFT_CORNER>({1500, 300, 1.f});
    EXPECT_NEAR(ndc_top_left.x, 0.5625f, 0.0001f);
    EXPECT_NEAR(ndc_top_left.y, 0.4444f, 0.0001f);
}

TEST(UnitTestProjection, ScreenToNdcBottomLeft)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(60.f);

    const auto cam = omath::unity_engine::Camera({0, 0, 0}, {}, {1280.f, 720.f}, fov, 0.03f, 1000.f);
    using ScreenStart = omath::unity_engine::Camera::ScreenStart;

    const auto ndc_bottom_left =
            cam.screen_to_ndc<ScreenStart::BOTTOM_LEFT_CORNER>({1263.53833f, 547.061523f, 0.99405992f});
    EXPECT_NEAR(ndc_bottom_left.x, 0.974278628f, 0.0001f);
    EXPECT_NEAR(ndc_bottom_left.y, 0.519615293f, 0.0001f);
}

TEST(UnitTestProjection, ScreenToWorldTopLeftCorner)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source

    std::uniform_real_distribution dist_x(1.f, 1900.f);
    std::uniform_real_distribution dist_y(1.f, 1070.f);

    constexpr auto fov = omath::Angle<float, 0.f, 180.f, omath::AngleFlags::Clamped>::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);
    using ScreenStart = omath::source_engine::Camera::ScreenStart;

    for (int i = 0; i < 100; i++)
    {
        const auto initial_screen_cords = omath::Vector2{dist_x(gen), dist_y(gen)};

        const auto world_cords = cam.screen_to_world<ScreenStart::TOP_LEFT_CORNER>(initial_screen_cords);
        const auto screen_cords = cam.world_to_screen<ScreenStart::TOP_LEFT_CORNER>(world_cords.value());

        EXPECT_NEAR(screen_cords->x, initial_screen_cords.x, 0.001f);
        EXPECT_NEAR(screen_cords->y, initial_screen_cords.y, 0.001f);
    }
}

TEST(UnitTestProjection, ScreenToWorldBottomLeftCorner)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source

    std::uniform_real_distribution dist_x(1.f, 1900.f);
    std::uniform_real_distribution dist_y(1.f, 1070.f);

    constexpr auto fov = omath::Angle<float, 0.f, 180.f, omath::AngleFlags::Clamped>::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);
    using ScreenStart = omath::source_engine::Camera::ScreenStart;

    for (int i = 0; i < 100; i++)
    {
        const auto initial_screen_cords = omath::Vector2{dist_x(gen), dist_y(gen)};

        const auto world_cords = cam.screen_to_world<ScreenStart::BOTTOM_LEFT_CORNER>(initial_screen_cords);
        const auto screen_cords = cam.world_to_screen<ScreenStart::BOTTOM_LEFT_CORNER>(world_cords.value());

        EXPECT_NEAR(screen_cords->x, initial_screen_cords.x, 0.001f);
        EXPECT_NEAR(screen_cords->y, initial_screen_cords.y, 0.001f);
    }
}