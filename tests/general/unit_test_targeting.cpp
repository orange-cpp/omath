//
// Created by claude on 19.03.2026.
//
#include <gtest/gtest.h>
#include <omath/algorithm/targeting.hpp>
#include <omath/engines/source_engine/camera.hpp>
#include <vector>

namespace
{
    using Camera = omath::source_engine::Camera;
    using ViewAngles = omath::source_engine::ViewAngles;
    using Targets = std::vector<omath::Vector3<float>>;
    using Iter = Targets::const_iterator;
    using FilterSig = bool(const omath::Vector3<float>&);

    constexpr auto k_fov = omath::Angle<float, 0.f, 180.f, omath::AngleFlags::Clamped>::from_degrees(90.f);

    Camera make_camera(const omath::Vector3<float>& origin, float pitch_deg, float yaw_deg)
    {
        ViewAngles angles{
                omath::source_engine::PitchAngle::from_degrees(pitch_deg),
                omath::source_engine::YawAngle::from_degrees(yaw_deg),
                omath::source_engine::RollAngle::from_degrees(0.f),
        };
        return Camera{origin, angles, {1920.f, 1080.f}, k_fov, 0.01f, 1000.f};
    }

    auto get_pos = [](const omath::Vector3<float>& v) -> const omath::Vector3<float>& { return v; };

    Iter find_closest(const Iter begin, const Iter end, const Camera& camera)
    {
        return omath::algorithm::get_closest_target_by_fov<Camera, Iter, FilterSig>(
                begin, end, camera, get_pos);
    }
}

TEST(unit_test_targeting, returns_end_for_empty_range)
{
    const auto camera = make_camera({0, 0, 0}, 0.f, 0.f);
    Targets targets;

    EXPECT_EQ(find_closest(targets.cbegin(), targets.cend(), camera), targets.cend());
}

TEST(unit_test_targeting, single_target_returns_that_target)
{
    const auto camera = make_camera({0, 0, 0}, 0.f, 0.f);
    Targets targets = {{100.f, 0.f, 0.f}};

    EXPECT_EQ(find_closest(targets.cbegin(), targets.cend(), camera), targets.cbegin());
}

TEST(unit_test_targeting, picks_closest_to_crosshair)
{
    // Camera looking forward along +X (yaw=0, pitch=0 in source engine)
    const auto camera = make_camera({0, 0, 0}, 0.f, 0.f);

    Targets targets = {
            {100.f, 50.f, 0.f},  // off to the side
            {100.f, 1.f, 0.f},   // nearly on crosshair
            {100.f, -30.f, 0.f}, // off to the other side
    };

    const auto result = find_closest(targets.cbegin(), targets.cend(), camera);

    ASSERT_NE(result, targets.cend());
    EXPECT_EQ(result, targets.cbegin() + 1);
}

TEST(unit_test_targeting, picks_closest_with_vertical_offset)
{
    const auto camera = make_camera({0, 0, 0}, 0.f, 0.f);

    Targets targets = {
            {100.f, 0.f, 50.f}, // high above
            {100.f, 0.f, 2.f},  // slightly above
            {100.f, 0.f, 30.f}, // moderately above
    };

    const auto result = find_closest(targets.cbegin(), targets.cend(), camera);

    ASSERT_NE(result, targets.cend());
    EXPECT_EQ(result, targets.cbegin() + 1);
}

TEST(unit_test_targeting, respects_camera_direction)
{
    // Camera looking along +Y (yaw=90)
    const auto camera = make_camera({0, 0, 0}, 0.f, 90.f);

    Targets targets = {
            {100.f, 0.f, 0.f}, // to the side relative to camera facing +Y
            {0.f, 100.f, 0.f}, // directly in front
    };

    const auto result = find_closest(targets.cbegin(), targets.cend(), camera);

    ASSERT_NE(result, targets.cend());
    EXPECT_EQ(result, targets.cbegin() + 1);
}

TEST(unit_test_targeting, equidistant_targets_returns_first)
{
    const auto camera = make_camera({0, 0, 0}, 0.f, 0.f);

    // Two targets symmetric about the forward axis — same angular distance
    Targets targets = {
            {100.f, 10.f, 0.f},
            {100.f, -10.f, 0.f},
    };

    const auto result = find_closest(targets.cbegin(), targets.cend(), camera);

    ASSERT_NE(result, targets.cend());
    // First target should be selected (strict < means first wins on tie)
    EXPECT_EQ(result, targets.cbegin());
}

TEST(unit_test_targeting, camera_pitch_affects_selection)
{
    // Camera looking upward (pitch < 0)
    const auto camera = make_camera({0, 0, 0}, -40.f, 0.f);

    Targets targets = {
            {100.f, 0.f, 0.f},   // on the horizon
            {100.f, 0.f, 40.f}, // above, closer to where camera is looking
    };

    const auto result = find_closest(targets.cbegin(), targets.cend(), camera);

    ASSERT_NE(result, targets.cend());
    EXPECT_EQ(result, targets.cbegin() + 1);
}

TEST(unit_test_targeting, many_targets_picks_best)
{
    const auto camera = make_camera({0, 0, 0}, 0.f, 0.f);

    Targets targets = {
            {100.f, 80.f, 80.f},
            {100.f, 60.f, 60.f},
            {100.f, 40.f, 40.f},
            {100.f, 20.f, 20.f},
            {100.f, 0.5f, 0.5f}, // closest to crosshair
            {100.f, 10.f, 10.f},
            {100.f, 30.f, 30.f},
    };

    const auto result = find_closest(targets.cbegin(), targets.cend(), camera);

    ASSERT_NE(result, targets.cend());
    EXPECT_EQ(result, targets.cbegin() + 4);
}
