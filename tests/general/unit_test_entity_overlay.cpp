//
// Created by orange on 30.06.2026.
//
#include <gtest/gtest.h>
#include <memory>
#include <omath/3d_primitives/aabb.hpp>
#include <omath/engines/unreal_engine/camera.hpp>
#include <omath/hud/entity_overlay.hpp>

TEST(EntityOverlayTests, FromAabbSupportsDoubleCameraCoordinates)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const omath::unreal_engine::Camera camera{{0.0, 0.0, 0.0}, {}, {1920.f, 1080.f}, fov, 0.01, 1000.0};
    const omath::primitives::Aabb<double> aabb{{90.0, -1.0, -1.0}, {110.0, 1.0, 1.0}};

    const auto overlay = omath::hud::EntityOverlay::from_aabb(camera, aabb, 0.5f,
                                                              std::shared_ptr<omath::hud::HudRendererInterface>{});

    ASSERT_TRUE(overlay.has_value());
}

TEST(EntityOverlayTests, FromAabbReturnsErrorCodeIfNoVertexProjects)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const omath::unreal_engine::Camera camera{{0.0, 0.0, 0.0}, {}, {1920.f, 1080.f}, fov, 0.01, 1000.0};
    const omath::primitives::Aabb<double> aabb{{-110.0, -1.0, -1.0}, {-90.0, 1.0, 1.0}};

    const auto overlay = omath::hud::EntityOverlay::from_aabb(camera, aabb, 0.5f,
                                                              std::shared_ptr<omath::hud::HudRendererInterface>{});

    ASSERT_FALSE(overlay.has_value());
    EXPECT_EQ(overlay.error(), omath::hud::EntityOverlayError::NO_PROJECTED_VERTEX);
}
