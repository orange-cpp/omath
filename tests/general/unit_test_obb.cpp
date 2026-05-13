//
// Created by Vladislav on 07.05.2026.
//
#include <cmath>
#include <gtest/gtest.h>
#include <numbers>
#include <omath/3d_primitives/obb.hpp>
#include <omath/engines/opengl_engine/camera.hpp>
#include <omath/engines/source_engine/camera.hpp>
#include <omath/engines/unity_engine/camera.hpp>
#include <omath/projection/camera.hpp>

using ObbF = omath::primitives::Obb<float>;
using ObbD = omath::primitives::Obb<double>;
using Vec3F = omath::Vector3<float>;
using Vec3D = omath::Vector3<double>;

namespace
{
    constexpr ObbF axis_aligned_obb(const Vec3F& center, const Vec3F& half_extents) noexcept
    {
        return ObbF{center, {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}, half_extents};
    }

    ObbF rotated_around_z(const Vec3F& center, const Vec3F& half_extents, const float radians) noexcept
    {
        const auto c = std::cos(radians);
        const auto s = std::sin(radians);
        return ObbF{center, {c, s, 0.f}, {-s, c, 0.f}, {0.f, 0.f, 1.f}, half_extents};
    }

    ObbF rotated_around_y(const Vec3F& center, const Vec3F& half_extents, const float radians) noexcept
    {
        const auto c = std::cos(radians);
        const auto s = std::sin(radians);
        return ObbF{center, {c, 0.f, -s}, {0.f, 1.f, 0.f}, {s, 0.f, c}, half_extents};
    }

    void expect_vec_near(const Vec3F& actual, const Vec3F& expected, const float epsilon = 1e-5f)
    {
        EXPECT_NEAR(actual.x, expected.x, epsilon);
        EXPECT_NEAR(actual.y, expected.y, epsilon);
        EXPECT_NEAR(actual.z, expected.z, epsilon);
    }
} // namespace

// --- struct-level tests ---

TEST(ObbTests, VerticesOfAxisAlignedUnitBox)
{
    constexpr auto box = axis_aligned_obb({0.f, 0.f, 0.f}, {1.f, 1.f, 1.f});
    constexpr auto v = box.vertices();

    EXPECT_EQ(v[0], (Vec3F{-1.f, -1.f, -1.f}));
    EXPECT_EQ(v[1], (Vec3F{1.f, -1.f, -1.f}));
    EXPECT_EQ(v[2], (Vec3F{-1.f, 1.f, -1.f}));
    EXPECT_EQ(v[3], (Vec3F{1.f, 1.f, -1.f}));
    EXPECT_EQ(v[4], (Vec3F{-1.f, -1.f, 1.f}));
    EXPECT_EQ(v[5], (Vec3F{1.f, -1.f, 1.f}));
    EXPECT_EQ(v[6], (Vec3F{-1.f, 1.f, 1.f}));
    EXPECT_EQ(v[7], (Vec3F{1.f, 1.f, 1.f}));
}

TEST(ObbTests, VerticesOfTranslatedBox)
{
    constexpr auto box = axis_aligned_obb({10.f, 20.f, 30.f}, {1.f, 2.f, 3.f});
    constexpr auto v = box.vertices();

    EXPECT_EQ(v[0], (Vec3F{9.f, 18.f, 27.f}));
    EXPECT_EQ(v[7], (Vec3F{11.f, 22.f, 33.f}));
}

TEST(ObbTests, VerticesOfRotatedBox)
{
    constexpr auto pi = std::numbers::pi_v<float>;
    const auto box = rotated_around_z({0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}, pi / 2.f);
    const auto v = box.vertices();

    // After 90° rotation around Z, local +X maps to world +Y, local +Y maps to world -X.
    // The eight vertices are still the same eight points (a cube is symmetric), but their
    // ordering changes. Check that the corner set as a whole is still |coord| == 1.
    for (const auto& corner : v)
    {
        EXPECT_NEAR(std::abs(corner.x), 1.f, 1e-5f);
        EXPECT_NEAR(std::abs(corner.y), 1.f, 1e-5f);
        EXPECT_NEAR(std::abs(corner.z), 1.f, 1e-5f);
    }
}

TEST(ObbTests, VerticesOfTranslatedNonUniformRotatedBox)
{
    const auto box = rotated_around_z({2.f, 3.f, 4.f}, {2.f, 1.f, 0.5f}, std::numbers::pi_v<float> / 2.f);
    const auto v = box.vertices();

    expect_vec_near(v[0], {3.f, 1.f, 3.5f});
    expect_vec_near(v[1], {3.f, 5.f, 3.5f});
    expect_vec_near(v[2], {1.f, 1.f, 3.5f});
    expect_vec_near(v[3], {1.f, 5.f, 3.5f});
    expect_vec_near(v[4], {3.f, 1.f, 4.5f});
    expect_vec_near(v[5], {3.f, 5.f, 4.5f});
    expect_vec_near(v[6], {1.f, 1.f, 4.5f});
    expect_vec_near(v[7], {1.f, 5.f, 4.5f});
}

TEST(ObbTests, VerticesCollapseWhenOneExtentIsZero)
{
    constexpr auto box = axis_aligned_obb({1.f, 2.f, 3.f}, {2.f, 0.f, 4.f});
    constexpr auto v = box.vertices();

    EXPECT_EQ(v[0], v[2]);
    EXPECT_EQ(v[1], v[3]);
    EXPECT_EQ(v[4], v[6]);
    EXPECT_EQ(v[5], v[7]);

    EXPECT_EQ(v[0], (Vec3F{-1.f, 2.f, -1.f}));
    EXPECT_EQ(v[1], (Vec3F{3.f, 2.f, -1.f}));
    EXPECT_EQ(v[4], (Vec3F{-1.f, 2.f, 7.f}));
    EXPECT_EQ(v[5], (Vec3F{3.f, 2.f, 7.f}));
}

TEST(ObbTests, DoublePrecisionInstantiation)
{
    constexpr ObbD box{{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}, {2.0, 3.0, 4.0}};
    constexpr auto v = box.vertices();
    EXPECT_DOUBLE_EQ(v[0].x, -2.0);
    EXPECT_DOUBLE_EQ(v[0].y, -3.0);
    EXPECT_DOUBLE_EQ(v[0].z, -4.0);
    EXPECT_DOUBLE_EQ(v[7].x, 2.0);
    EXPECT_DOUBLE_EQ(v[7].y, 3.0);
    EXPECT_DOUBLE_EQ(v[7].z, 4.0);
}

// --- frustum culling tests (Source Engine: +X forward, +Y left, +Z up) ---

TEST(ObbTests, AxisAlignedInFrontNotCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    constexpr auto obb = axis_aligned_obb({100.f, 0.f, 0.f}, {10.f, 1.f, 1.f});
    EXPECT_FALSE(cam.is_obb_culled_by_frustum(obb));
}

TEST(ObbTests, AxisAlignedBehindCameraCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    constexpr auto obb = axis_aligned_obb({-150.f, 0.f, 0.f}, {50.f, 1.f, 1.f});
    EXPECT_TRUE(cam.is_obb_culled_by_frustum(obb));
}

TEST(ObbTests, AxisAlignedBeyondFarPlaneCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    constexpr auto obb = axis_aligned_obb({1750.f, 0.f, 0.f}, {250.f, 1.f, 1.f});
    EXPECT_TRUE(cam.is_obb_culled_by_frustum(obb));
}

TEST(ObbTests, AxisAlignedStraddlingFarPlaneNotCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    constexpr auto obb = axis_aligned_obb({1005.f, 0.f, 0.f}, {10.f, 1.f, 1.f});
    EXPECT_FALSE(cam.is_obb_culled_by_frustum(obb));
}

TEST(ObbTests, AxisAlignedFarLeftCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    constexpr auto obb = axis_aligned_obb({100.f, 4500.f, 0.f}, {10.f, 500.f, 1.f});
    EXPECT_TRUE(cam.is_obb_culled_by_frustum(obb));
}

TEST(ObbTests, AxisAlignedFarRightCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    constexpr auto obb = axis_aligned_obb({100.f, -4500.f, 0.f}, {10.f, 500.f, 1.f});
    EXPECT_TRUE(cam.is_obb_culled_by_frustum(obb));
}

TEST(ObbTests, AxisAlignedAboveCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    constexpr auto obb = axis_aligned_obb({100.f, 0.f, 5500.f}, {10.f, 1.f, 500.f});
    EXPECT_TRUE(cam.is_obb_culled_by_frustum(obb));
}

TEST(ObbTests, AxisAlignedBelowCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    constexpr auto obb = axis_aligned_obb({100.f, 0.f, -5500.f}, {10.f, 1.f, 500.f});
    EXPECT_TRUE(cam.is_obb_culled_by_frustum(obb));
}

TEST(ObbTests, MatchesAabbForAxisAlignedBox)
{
    // For axis-aligned OBBs, the result must agree with is_aabb_culled_by_frustum.
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    const struct
    {
        Vec3F center;
        Vec3F half;
    } cases[] = {
            {{100.f, 0.f, 0.f}, {10.f, 1.f, 1.f}}, // in front
            {{-150.f, 0.f, 0.f}, {50.f, 1.f, 1.f}}, // behind
            {{1750.f, 0.f, 0.f}, {250.f, 1.f, 1.f}}, // beyond far
            {{100.f, 4500.f, 0.f}, {10.f, 500.f, 1.f}}, // far left
            {{0.f, 0.f, 0.f}, {500.f, 500.f, 500.f}}, // encloses camera
            {{275.f, 0.f, 0.f}, {225.f, 1.f, 1.f}}, // straddles near
    };

    for (const auto& [center, half]: cases)
    {
        const omath::primitives::Aabb<float> aabb{center - half, center + half};
        const auto obb = axis_aligned_obb(center, half);
        EXPECT_EQ(cam.is_obb_culled_by_frustum(obb), cam.is_aabb_culled_by_frustum(aabb))
                << "mismatch for center (" << center.x << "," << center.y << "," << center.z << ")";
    }
}

TEST(ObbTests, RotationCanPullBoxIntoFrustum)
{
    // Tall thin column sitting just outside the +Y frustum boundary at X=50.
    // Axis-aligned: every corner has Y≈100 at X≈50, all outside the +Y plane → culled.
    // Rotated 90° around world Y: the 50-unit extent now points along world +X, so the rod
    // sweeps forward to X≈100 where the +Y plane is far more permissive — front end inside,
    // box no longer fully outside → not culled.
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    constexpr Vec3F center{50.f, 100.f, 0.f};
    constexpr Vec3F half{1.f, 1.f, 50.f};

    constexpr auto axis_aligned = axis_aligned_obb(center, half);
    EXPECT_TRUE(cam.is_obb_culled_by_frustum(axis_aligned));

    const auto rotated = rotated_around_y(center, half, std::numbers::pi_v<float> / 2.f);
    EXPECT_FALSE(cam.is_obb_culled_by_frustum(rotated));
}

TEST(ObbTests, RotationCanPushBoxOutOfFrustum)
{
    // Long forward-pointing rod whose front end pokes into the frustum near the +Y boundary.
    // Axis-aligned (long along X): the front end at X≈100 has Y=129 just inside the +Y plane,
    // so part of the rod is visible → not culled.
    // Rotated 90° around Z: the rod's long axis now points along world Y, so all corners
    // shift to Y∈[80,180] at X≈50 — every corner is outside the +Y plane → culled.
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    constexpr Vec3F center{50.f, 130.f, 0.f};
    constexpr Vec3F half{50.f, 1.f, 1.f};

    constexpr auto axis_aligned = axis_aligned_obb(center, half);
    EXPECT_FALSE(cam.is_obb_culled_by_frustum(axis_aligned));

    const auto rotated = rotated_around_z(center, half, std::numbers::pi_v<float> / 2.f);
    EXPECT_TRUE(cam.is_obb_culled_by_frustum(rotated));
}

TEST(ObbTests, RotatedBoxStraddlingFrustumNotCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Box centred in front, rotated 30° — clearly straddles into the frustum.
    const auto obb = rotated_around_z({200.f, 0.f, 0.f}, {50.f, 50.f, 50.f},
                                      std::numbers::pi_v<float> / 6.f);
    EXPECT_FALSE(cam.is_obb_culled_by_frustum(obb));
}

TEST(ObbTests, OpenGlEngineRotatedInFrontNotCulled)
{
    // OpenGL: -Z forward, COLUMN_MAJOR, NEGATIVE_ONE_TO_ONE
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    const auto obb = rotated_around_z({0.f, 0.f, -100.f}, {5.f, 5.f, 5.f},
                                      std::numbers::pi_v<float> / 4.f);
    EXPECT_FALSE(cam.is_obb_culled_by_frustum(obb));
}

TEST(ObbTests, OpenGlEngineBehindCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    constexpr auto obb = axis_aligned_obb({0.f, 0.f, 100.f}, {5.f, 5.f, 5.f});
    EXPECT_TRUE(cam.is_obb_culled_by_frustum(obb));
}

TEST(ObbTests, OpenGlEngineStraddlingFarPlaneNotCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 100.f);

    const auto obb = rotated_around_z({0.f, 0.f, -105.f}, {5.f, 5.f, 10.f},
                                      std::numbers::pi_v<float> / 4.f);
    EXPECT_FALSE(cam.is_obb_culled_by_frustum(obb));
}

TEST(ObbTests, UnityEngineBeyondFarCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(60.f);
    const auto cam = omath::unity_engine::Camera({0, 0, 0}, {}, {1280.f, 720.f}, fov, 0.03f, 500.f);

    const auto obb = rotated_around_z({0.f, 0.f, 700.f}, {5.f, 5.f, 5.f},
                                      std::numbers::pi_v<float> / 4.f);
    EXPECT_TRUE(cam.is_obb_culled_by_frustum(obb));
}

TEST(ObbTests, DegenerateZeroVolumeInsideNotCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Zero-extent OBB — collapses to a point, but still must not be culled if the centre is inside.
    constexpr auto obb = axis_aligned_obb({100.f, 0.f, 0.f}, {0.f, 0.f, 0.f});
    EXPECT_FALSE(cam.is_obb_culled_by_frustum(obb));
}

TEST(ObbTests, EnclosingCameraNotCulled)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, fov,
                                                  0.01f, 1000.f);

    // Huge rotated box that fully encloses the camera origin.
    const auto obb = rotated_around_z({0.f, 0.f, 0.f}, {500.f, 500.f, 500.f},
                                      std::numbers::pi_v<float> / 5.f);
    EXPECT_FALSE(cam.is_obb_culled_by_frustum(obb));
}
