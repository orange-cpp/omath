//
// Created by Vlad on 27.08.2024.
//
#include "omath/engines/unity_engine/camera.hpp"
#include <complex>
#include <gtest/gtest.h>
#include <omath/3d_primitives/aabb.hpp>
#include <omath/engines/opengl_engine/camera.hpp>
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

TEST(UnitTestProjection, UnclippedWorldToScreenInBounds)
{
    constexpr auto fov = omath::Angle<float, 0.f, 180.f, omath::AngleFlags::Clamped>::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    const auto projected = cam.world_to_screen_unclipped({1000.f, 0, 50.f});
    ASSERT_TRUE(projected.has_value());
    EXPECT_NEAR(projected->x, 960.f, 0.001f);
    EXPECT_NEAR(projected->y, 504.f, 0.001f);
}

TEST(UnitTestProjection, UnclippedWorldToScreenMatchesWorldToScreenWhenInBounds)
{
    constexpr auto fov = omath::Angle<float, 0.f, 180.f, omath::AngleFlags::Clamped>::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    const auto w2s = cam.world_to_screen({1000.f, 0, 50.f});
    const auto no_clip = cam.world_to_screen_unclipped({1000.f, 0, 50.f});

    ASSERT_TRUE(w2s.has_value());
    ASSERT_TRUE(no_clip.has_value());
    EXPECT_NEAR(w2s->x, no_clip->x, 0.001f);
    EXPECT_NEAR(w2s->y, no_clip->y, 0.001f);
    EXPECT_NEAR(w2s->z, no_clip->z, 0.001f);
}

TEST(UnitTestProjection, UnclippedWorldToScreenRejectsBehindCamera)
{
    constexpr auto fov = omath::Angle<float, 0.f, 180.f, omath::AngleFlags::Clamped>::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    const auto projected = cam.world_to_screen_unclipped({-1000.f, 0, 0});
    EXPECT_FALSE(projected.has_value());
    EXPECT_EQ(projected.error(), omath::projection::Error::PERSPECTIVE_DIVIDER_LESS_EQ_ZERO);
}

TEST(UnitTestProjection, UnclippedWorldToScreenAllowsOutOfBoundsNdc)
{
    constexpr auto fov = omath::Angle<float, 0.f, 180.f, omath::AngleFlags::Clamped>::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Point far to the side exceeds NDC [-1,1] bounds but unclipped returns it anyway
    const auto projected = cam.world_to_screen_unclipped({100.f, 5000.f, 0});
    EXPECT_TRUE(projected.has_value());
}

TEST(UnitTestProjection, WorldToScreenRejectsOutOfBoundsNdc)
{
    constexpr auto fov = omath::Angle<float, 0.f, 180.f, omath::AngleFlags::Clamped>::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Same point that unclipped allows — clipped world_to_screen rejects it
    const auto projected = cam.world_to_screen({100.f, 5000.f, 0});
    EXPECT_FALSE(projected.has_value());
}

TEST(UnitTestProjection, UnclippedWorldToScreenBottomLeftCorner)
{
    constexpr auto fov = omath::Angle<float, 0.f, 180.f, omath::AngleFlags::Clamped>::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);
    using ScreenStart = omath::source_engine::Camera::ScreenStart;

    const auto top_left = cam.world_to_screen_unclipped<ScreenStart::TOP_LEFT_CORNER>({1000.f, 0, 50.f});
    const auto bottom_left = cam.world_to_screen_unclipped<ScreenStart::BOTTOM_LEFT_CORNER>({1000.f, 0, 50.f});

    ASSERT_TRUE(top_left.has_value());
    ASSERT_TRUE(bottom_left.has_value());
    // X should be identical, Y should differ (mirrored around center)
    EXPECT_NEAR(top_left->x, bottom_left->x, 0.001f);
    EXPECT_NEAR(top_left->y + bottom_left->y, 1080.f, 0.001f);
}

TEST(UnitTestProjection, UnclippedWorldToScreenRoundTrip)
{
    std::mt19937 gen(42);
    std::uniform_real_distribution dist_fwd(100.f, 900.f);
    std::uniform_real_distribution dist_side(-400.f, 400.f);
    std::uniform_real_distribution dist_up(-200.f, 200.f);

    constexpr auto fov = omath::Angle<float, 0.f, 180.f, omath::AngleFlags::Clamped>::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    for (int i = 0; i < 100; i++)
    {
        const omath::Vector3<float> world_pos{dist_fwd(gen), dist_side(gen), dist_up(gen)};
        const auto screen = cam.world_to_screen_unclipped(world_pos);
        if (!screen.has_value())
            continue;

        const auto back_to_world = cam.screen_to_world(screen.value());
        ASSERT_TRUE(back_to_world.has_value());

        const auto back_to_screen = cam.world_to_screen_unclipped(back_to_world.value());
        ASSERT_TRUE(back_to_screen.has_value());

        EXPECT_NEAR(screen->x, back_to_screen->x, 0.01f);
        EXPECT_NEAR(screen->y, back_to_screen->y, 0.01f);
    }
}

TEST(UnitTestProjection, UnclippedWorldToScreenUnityEngine)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(60.f);
    const auto cam = omath::unity_engine::Camera({0, 0, 0}, {}, {1280.f, 720.f}, fov, 0.03f, 1000.f);
    using ScreenStart = omath::unity_engine::Camera::ScreenStart;

    // Point directly in front
    const auto projected = cam.world_to_screen_unclipped<ScreenStart::BOTTOM_LEFT_CORNER>({0, 0, 500.f});
    ASSERT_TRUE(projected.has_value());
    EXPECT_NEAR(projected->x, 640.f, 0.5f);
    EXPECT_NEAR(projected->y, 360.f, 0.5f);
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

TEST(UnitTestProjection, AabbInsideFrustumNotCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Small box directly in front of camera (Source Engine: +X forward, +Y left, +Z up)
    const omath::primitives::Aabb<float> aabb{{90.f, -1.f, -1.f}, {110.f, 1.f, 1.f}};
    EXPECT_FALSE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbBehindCameraCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Box entirely behind the camera
    const omath::primitives::Aabb<float> aabb{{-200.f, -1.f, -1.f}, {-100.f, 1.f, 1.f}};
    EXPECT_TRUE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbBeyondFarPlaneCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Box beyond far plane (1000)
    const omath::primitives::Aabb<float> aabb{{1500.f, -1.f, -1.f}, {2000.f, 1.f, 1.f}};
    EXPECT_TRUE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbFarLeftCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Box far to the side, outside the frustum
    const omath::primitives::Aabb<float> aabb{{90.f, 4000.f, -1.f}, {110.f, 5000.f, 1.f}};
    EXPECT_TRUE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbFarRightCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Box far to the other side, outside the frustum
    const omath::primitives::Aabb<float> aabb{{90.f, -5000.f, -1.f}, {110.f, -4000.f, 1.f}};
    EXPECT_TRUE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbAboveCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Box far above the frustum
    const omath::primitives::Aabb<float> aabb{{90.f, -1.f, 5000.f}, {110.f, 1.f, 6000.f}};
    EXPECT_TRUE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbPartiallyInsideNotCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Large box that straddles the frustum boundary — partially inside
    const omath::primitives::Aabb<float> aabb{{50.f, -5000.f, -5000.f}, {500.f, 5000.f, 5000.f}};
    EXPECT_FALSE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbStraddlesNearPlaneNotCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Box that straddles the near plane — partially in front
    const omath::primitives::Aabb<float> aabb{{-5.f, -1.f, -1.f}, {5.f, 1.f, 1.f}};
    EXPECT_FALSE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbStraddlesFarPlaneNotCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Box that straddles the far plane
    const omath::primitives::Aabb<float> aabb{{900.f, -1.f, -1.f}, {1100.f, 1.f, 1.f}};
    EXPECT_FALSE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbCulledUnityEngine)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(60.f);
    const auto cam = omath::unity_engine::Camera({0, 0, 0}, {}, {1280.f, 720.f}, fov, 0.03f, 1000.f);

    // Box in front — not culled
    const omath::primitives::Aabb<float> inside{{-1.f, -1.f, 50.f}, {1.f, 1.f, 100.f}};
    EXPECT_FALSE(cam.is_aabb_culled_by_frustum(inside));

    // Box behind — culled
    const omath::primitives::Aabb<float> behind{{-1.f, -1.f, -200.f}, {1.f, 1.f, -100.f}};
    EXPECT_TRUE(cam.is_aabb_culled_by_frustum(behind));
}

TEST(UnitTestProjection, AabbBelowCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Box far below the frustum (Source Engine: +Z up)
    const omath::primitives::Aabb<float> aabb{{90.f, -1.f, -6000.f}, {110.f, 1.f, -5000.f}};
    EXPECT_TRUE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbEnclosesCameraNotCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Huge box that fully encloses the camera
    const omath::primitives::Aabb<float> aabb{{-500.f, -500.f, -500.f}, {500.f, 500.f, 500.f}};
    EXPECT_FALSE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbExactlyAtNearPlaneNotCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Box starting exactly at the near plane distance
    const omath::primitives::Aabb<float> aabb{{0.01f, -1.f, -1.f}, {10.f, 1.f, 1.f}};
    EXPECT_FALSE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbExactlyAtFarPlaneNotCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Box ending exactly at the far plane distance
    const omath::primitives::Aabb<float> aabb{{990.f, -1.f, -1.f}, {1000.f, 1.f, 1.f}};
    EXPECT_FALSE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbNarrowFovCulledAtEdge)
{
    // Narrow FOV — box that would be visible at 90 is culled at 30
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(30.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    const omath::primitives::Aabb<float> aabb{{100.f, 200.f, -1.f}, {110.f, 210.f, 1.f}};
    EXPECT_TRUE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbWideFovNotCulledAtEdge)
{
    // Wide FOV — same box is visible
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(120.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    const omath::primitives::Aabb<float> aabb{{100.f, 200.f, -1.f}, {110.f, 210.f, 1.f}};
    EXPECT_FALSE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbCameraOffOrigin)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({500.f, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f},
                                                  fov, 0.01f, 1000.f);

    // Box in front of shifted camera
    const omath::primitives::Aabb<float> in_front{{600.f, -1.f, -1.f}, {700.f, 1.f, 1.f}};
    EXPECT_FALSE(cam.is_aabb_culled_by_frustum(in_front));

    // Box behind shifted camera
    const omath::primitives::Aabb<float> behind{{0.f, -1.f, -1.f}, {100.f, 1.f, 1.f}};
    EXPECT_TRUE(cam.is_aabb_culled_by_frustum(behind));
}

TEST(UnitTestProjection, AabbShortFarPlaneCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    // Very short far plane
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 50.f);

    // Box at distance 100 — beyond the 50-unit far plane
    const omath::primitives::Aabb<float> aabb{{90.f, -1.f, -1.f}, {110.f, 1.f, 1.f}};
    EXPECT_TRUE(cam.is_aabb_culled_by_frustum(aabb));

    // Box at distance 30 — within range
    const omath::primitives::Aabb<float> near_box{{25.f, -1.f, -1.f}, {35.f, 1.f, 1.f}};
    EXPECT_FALSE(cam.is_aabb_culled_by_frustum(near_box));
}

TEST(UnitTestProjection, AabbPointSizedInsideNotCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Degenerate zero-volume AABB (a point) inside the frustum
    const omath::primitives::Aabb<float> aabb{{100.f, 0.f, 0.f}, {100.f, 0.f, 0.f}};
    EXPECT_FALSE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbOpenGlEngineInsideNotCulled)
{
    // OpenGL: COLUMN_MAJOR, NEGATIVE_ONE_TO_ONE, inverted_z, forward = -Z
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    // Box in front of camera (OpenGL: -Z forward)
    const omath::primitives::Aabb<float> aabb{{-1.f, -1.f, -110.f}, {1.f, 1.f, -90.f}};
    EXPECT_FALSE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbOpenGlEngineBehindCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    // Box behind (OpenGL: +Z is behind the camera)
    const omath::primitives::Aabb<float> aabb{{-1.f, -1.f, 100.f}, {1.f, 1.f, 200.f}};
    EXPECT_TRUE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbOpenGlEngineBeyondFarCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    // Box beyond far plane along -Z
    const omath::primitives::Aabb<float> aabb{{-1.f, -1.f, -2000.f}, {1.f, 1.f, -1500.f}};
    EXPECT_TRUE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbOpenGlEngineSideCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    // Box far to the right (OpenGL: +X right)
    const omath::primitives::Aabb<float> aabb{{4000.f, -1.f, -110.f}, {5000.f, 1.f, -90.f}};
    EXPECT_TRUE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbUnityEngineBeyondFarCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(60.f);
    const auto cam = omath::unity_engine::Camera({0, 0, 0}, {}, {1280.f, 720.f}, fov, 0.03f, 500.f);

    // Box beyond 500-unit far plane (Unity: +Z forward)
    const omath::primitives::Aabb<float> aabb{{-1.f, -1.f, 600.f}, {1.f, 1.f, 700.f}};
    EXPECT_TRUE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbUnityEngineSideCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(60.f);
    const auto cam = omath::unity_engine::Camera({0, 0, 0}, {}, {1280.f, 720.f}, fov, 0.03f, 1000.f);

    // Box far above (Unity: +Y up)
    const omath::primitives::Aabb<float> aabb{{-1.f, 5000.f, 50.f}, {1.f, 6000.f, 100.f}};
    EXPECT_TRUE(cam.is_aabb_culled_by_frustum(aabb));
}

TEST(UnitTestProjection, AabbUnityEngineStraddlesNearNotCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(60.f);
    const auto cam = omath::unity_engine::Camera({0, 0, 0}, {}, {1280.f, 720.f}, fov, 0.03f, 1000.f);

    // Box straddles near plane (Unity: +Z forward)
    const omath::primitives::Aabb<float> aabb{{-1.f, -1.f, -5.f}, {1.f, 1.f, 5.f}};
    EXPECT_FALSE(cam.is_aabb_culled_by_frustum(aabb));
}