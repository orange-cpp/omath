//
// Created by Vlad on 10/23/2025.
//
#include <gtest/gtest.h>
#include <omath/engines/frostbite_engine/camera.hpp>
#include <omath/engines/frostbite_engine/constants.hpp>
#include <omath/engines/frostbite_engine/formulas.hpp>
#include <print>
#include <random>
#include <ranges>

TEST(unit_test_frostbite_engine, UnitsToCentimeters_BasicValues)
{
    EXPECT_FLOAT_EQ(omath::frostbite_engine::units_to_centimeters(0.0f), 0.0f);
    EXPECT_FLOAT_EQ(omath::frostbite_engine::units_to_centimeters(1.0f), 0.01f);
    EXPECT_FLOAT_EQ(omath::frostbite_engine::units_to_centimeters(100.0f), 1.0f);
    EXPECT_FLOAT_EQ(omath::frostbite_engine::units_to_centimeters(-250.0f), -2.5f);
}

TEST(unit_test_frostbite_engine, UnitsToMeters_BasicValues)
{
    EXPECT_DOUBLE_EQ(omath::frostbite_engine::units_to_meters(0.0), 0.0);
    EXPECT_DOUBLE_EQ(omath::frostbite_engine::units_to_meters(1.0), 1.0);
    EXPECT_DOUBLE_EQ(omath::frostbite_engine::units_to_meters(123.456), 123.456);
    EXPECT_DOUBLE_EQ(omath::frostbite_engine::units_to_meters(-42.0), -42.0);
}

TEST(unit_test_frostbite_engine, UnitsToKilometers_BasicValues)
{
    EXPECT_NEAR(omath::frostbite_engine::units_to_kilometers(0.0), 0.0, 1e-15);
    EXPECT_NEAR(omath::frostbite_engine::units_to_kilometers(1.0), 0.001, 1e-15);
    EXPECT_NEAR(omath::frostbite_engine::units_to_kilometers(1000.0), 1.0, 1e-12);
    EXPECT_NEAR(omath::frostbite_engine::units_to_kilometers(-2500.0), -2.5, 1e-12);
}

TEST(unit_test_frostbite_engine, CentimetersToUnits_BasicValues)
{
    EXPECT_FLOAT_EQ(omath::frostbite_engine::centimeters_to_units(0.0f), 0.0f);
    EXPECT_FLOAT_EQ(omath::frostbite_engine::centimeters_to_units(0.01f), 1.0f);
    EXPECT_FLOAT_EQ(omath::frostbite_engine::centimeters_to_units(1.0f), 100.0f);
    EXPECT_FLOAT_EQ(omath::frostbite_engine::centimeters_to_units(-2.5f), -250.0f);
}

TEST(unit_test_frostbite_engine, MetersToUnits_BasicValues)
{
    EXPECT_DOUBLE_EQ(omath::frostbite_engine::meters_to_units(0.0), 0.0);
    EXPECT_DOUBLE_EQ(omath::frostbite_engine::meters_to_units(1.0), 1.0);
    EXPECT_DOUBLE_EQ(omath::frostbite_engine::meters_to_units(123.456), 123.456);
    EXPECT_DOUBLE_EQ(omath::frostbite_engine::meters_to_units(-42.0), -42.0);
}

TEST(unit_test_frostbite_engine, KilometersToUnits_BasicValues)
{
    EXPECT_NEAR(omath::frostbite_engine::kilometers_to_units(0.0), 0.0, 1e-12);
    EXPECT_NEAR(omath::frostbite_engine::kilometers_to_units(0.001), 1.0, 1e-12);
    EXPECT_NEAR(omath::frostbite_engine::kilometers_to_units(1.0), 1000.0, 1e-9);
    EXPECT_NEAR(omath::frostbite_engine::kilometers_to_units(-2.5), -2500.0, 1e-9);
}

TEST(unit_test_frostbite_engine, RoundTrip_UnitsCentimeters)
{
    constexpr float units_f = 12345.678f;
    const auto cm_f = omath::frostbite_engine::units_to_centimeters(units_f);
    const auto units_f_back = omath::frostbite_engine::centimeters_to_units(cm_f);
    EXPECT_NEAR(units_f_back, units_f, 1e-3f);

    constexpr double units_d = -987654.321;
    const auto cm_d = omath::frostbite_engine::units_to_centimeters(units_d);
    const auto units_d_back = omath::frostbite_engine::centimeters_to_units(cm_d);
    EXPECT_NEAR(units_d_back, units_d, 1e-9);
}

TEST(unit_test_frostbite_engine, RoundTrip_UnitsMeters)
{
    constexpr float units_f = 5432.125f;
    constexpr auto m_f = omath::frostbite_engine::units_to_meters(units_f);
    constexpr auto units_f_back = omath::frostbite_engine::meters_to_units(m_f);
    EXPECT_FLOAT_EQ(units_f_back, units_f);

    constexpr double units_d = -123456.789;
    constexpr auto m_d = omath::frostbite_engine::units_to_meters(units_d);
    constexpr auto units_d_back = omath::frostbite_engine::meters_to_units(m_d);
    EXPECT_DOUBLE_EQ(units_d_back, units_d);
}

TEST(unit_test_frostbite_engine, RoundTrip_UnitsKilometers)
{
    constexpr float units_f = 100000.0f;
    constexpr auto km_f = omath::frostbite_engine::units_to_kilometers(units_f);
    constexpr auto units_f_back = omath::frostbite_engine::kilometers_to_units(km_f);
    EXPECT_NEAR(units_f_back, units_f, 1e-2f);

    constexpr double units_d = -7654321.123;
    constexpr auto km_d = omath::frostbite_engine::units_to_kilometers(units_d);
    constexpr auto units_d_back = omath::frostbite_engine::kilometers_to_units(km_d);
    EXPECT_NEAR(units_d_back, units_d, 1e-6);
}

TEST(unit_test_frostbite_engine, ConversionChainConsistency)
{
    const double units = 424242.42;

    const auto cm_direct = omath::frostbite_engine::units_to_centimeters(units);
    const auto cm_via_units = units / 100.0;
    EXPECT_NEAR(cm_direct, cm_via_units, 1e-12);

    const auto km_direct = omath::frostbite_engine::units_to_kilometers(units);
    const auto km_via_meters = omath::frostbite_engine::units_to_meters(units) / 1000.0;
    EXPECT_NEAR(km_direct, km_via_meters, 1e-12);
}

TEST(unit_test_frostbite_engine, SupportsFloatAndDouble)
{
    static_assert(std::is_same_v<decltype(omath::frostbite_engine::units_to_centimeters(1.0f)), float>);
    static_assert(std::is_same_v<decltype(omath::frostbite_engine::units_to_centimeters(1.0)), double>);
    static_assert(std::is_same_v<decltype(omath::frostbite_engine::meters_to_units(1.0f)), float>);
    static_assert(std::is_same_v<decltype(omath::frostbite_engine::kilometers_to_units(1.0)), double>);
}

TEST(unit_test_frostbite_engine, ConstexprConversions)
{
    constexpr double units = 1000.0;
    constexpr double cm = omath::frostbite_engine::units_to_centimeters(units);
    constexpr double m = omath::frostbite_engine::units_to_meters(units);
    constexpr double km = omath::frostbite_engine::units_to_kilometers(units);

    static_assert(cm == 10.0, "units_to_centimeters constexpr failed");
    static_assert(m == 1000.0, "units_to_meters constexpr failed");
    static_assert(km == 1.0, "units_to_kilometers constexpr failed");
}
TEST(unit_test_frostbite_engine, ForwardVector)
{
    const auto forward = omath::frostbite_engine::forward_vector({});

    EXPECT_EQ(forward, omath::frostbite_engine::k_abs_forward);
}

TEST(unit_test_frostbite_engine, ForwardVectorRotationYaw)
{
    omath::frostbite_engine::ViewAngles angles;

    angles.yaw = omath::frostbite_engine::YawAngle::from_degrees(90.f);

    const auto forward = omath::frostbite_engine::forward_vector(angles);
    EXPECT_NEAR(forward.x, omath::frostbite_engine::k_abs_right.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::frostbite_engine::k_abs_right.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::frostbite_engine::k_abs_right.z, 0.00001f);
}

TEST(unit_test_frostbite_engine, ForwardVectorRotationPitch)
{
    omath::frostbite_engine::ViewAngles angles;

    angles.pitch = omath::frostbite_engine::PitchAngle::from_degrees(-90.f);

    const auto forward = omath::frostbite_engine::forward_vector(angles);
    EXPECT_NEAR(forward.x, omath::frostbite_engine::k_abs_up.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::frostbite_engine::k_abs_up.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::frostbite_engine::k_abs_up.z, 0.00001f);
}

TEST(unit_test_frostbite_engine, ForwardVectorRotationRoll)
{
    omath::frostbite_engine::ViewAngles angles;

    angles.roll = omath::frostbite_engine::RollAngle::from_degrees(-90.f);

    const auto forward = omath::frostbite_engine::up_vector(angles);
    EXPECT_NEAR(forward.x, omath::frostbite_engine::k_abs_right.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::frostbite_engine::k_abs_right.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::frostbite_engine::k_abs_right.z, 0.00001f);
}

TEST(unit_test_frostbite_engine, RightVector)
{
    const auto right = omath::frostbite_engine::right_vector({});

    EXPECT_EQ(right, omath::frostbite_engine::k_abs_right);
}

TEST(unit_test_frostbite_engine, UpVector)
{
    const auto up = omath::frostbite_engine::up_vector({});
    EXPECT_EQ(up, omath::frostbite_engine::k_abs_up);
}

TEST(unit_test_frostbite_engine, ProjectTargetMovedFromCamera)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(60.f);
    const auto cam = omath::frostbite_engine::Camera({0, 0, 0}, {}, {1280.f, 720.f}, fov, 0.01f, 1000.f);

    for (float distance = 0.02f; distance < 100.f; distance += 0.01f)
    {
        const auto projected = cam.world_to_screen({0, 0, distance});

        EXPECT_TRUE(projected.has_value());

        if (!projected.has_value())
            continue;

        EXPECT_NEAR(projected->x, 640, 0.00001f);
        EXPECT_NEAR(projected->y, 360, 0.00001f);
    }
}
TEST(unit_test_frostbite_engine, Project)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(60.f);

    const auto cam = omath::frostbite_engine::Camera({0, 0, 0}, {}, {1280.f, 720.f}, fov, 0.03f, 1000.f);
    const auto proj = cam.world_to_screen<omath::frostbite_engine::Camera::ScreenStart::BOTTOM_LEFT_CORNER>({10.f, 3, 10.f});

    EXPECT_NEAR(proj->x, 1263.538, 0.001f);
    EXPECT_NEAR(proj->y, 547.061f, 0.001f);
}

TEST(unit_test_frostbite_engine, CameraSetAndGetFov)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::frostbite_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 90.f);
    cam.set_field_of_view(omath::projection::FieldOfView::from_degrees(50.f));

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 50.f);
}

TEST(unit_test_frostbite_engine, CameraSetAndGetOrigin)
{
    auto cam = omath::frostbite_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, {}, 0.01f, 1000.f);

    EXPECT_EQ(cam.get_origin(), omath::Vector3<float>{});
    cam.set_field_of_view(omath::projection::FieldOfView::from_degrees(50.f));

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 50.f);
}
TEST(unit_test_frostbite_engine, loook_at_random_all_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::frostbite_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);

    std::size_t failed_points = 0;

    for (int i = 0; i < 1000; i++)
    {
        const auto position_to_look = omath::Vector3<float>{dist(gen), dist(gen), dist(gen)};

        if (cam.get_origin().distance_to(position_to_look) < 10)
            continue;

        cam.look_at(position_to_look);

        auto projected_pos = cam.world_to_view_port(position_to_look);

        EXPECT_TRUE(projected_pos.has_value());

        if (!projected_pos)
            continue;

        if (std::abs(projected_pos->x - 0.f) >= 0.0001f || std::abs(projected_pos->y - 0.f) >= 0.0001f)
            failed_points++;
    }
    EXPECT_LE(failed_points, 100);
}

TEST(unit_test_frostbite_engine, loook_at_random_x_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::frostbite_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);

    std::size_t failed_points = 0;
    for (int i = 0; i < 1000; i++)
    {
        const auto position_to_look = omath::Vector3<float>{dist(gen), 0.f, 0.f};
        if (cam.get_origin().distance_to(position_to_look) < 10)
            continue;

        cam.look_at(position_to_look);

        auto projected_pos = cam.world_to_view_port(position_to_look);

        EXPECT_TRUE(projected_pos.has_value());

        if (!projected_pos)
            continue;

        if (std::abs(projected_pos->x - 0.f) >= 0.001f || std::abs(projected_pos->y - 0.f) >= 0.001f)
            failed_points++;
    }
    EXPECT_LE(failed_points, 100);
}

TEST(unit_test_frostbite_engine, loook_at_random_y_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::frostbite_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);

    std::size_t failed_points = 0;
    for (int i = 0; i < 1000; i++)
    {
        const auto position_to_look = omath::Vector3<float>{0.f, dist(gen), 0.f};
        if (cam.get_origin().distance_to(position_to_look) < 10)
            continue;

        cam.look_at(position_to_look);

        auto projected_pos = cam.world_to_view_port(position_to_look);

        EXPECT_TRUE(projected_pos.has_value());

        if (!projected_pos)
            continue;

        if (std::abs(projected_pos->x - 0.f) >= 0.01f || std::abs(projected_pos->y - 0.f) >= 0.01f)
            failed_points++;
    }
    EXPECT_LE(failed_points, 100);
}

TEST(unit_test_frostbite_engine, loook_at_random_z_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::frostbite_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);

    std::size_t failed_points = 0;
    for (int i = 0; i < 1000; i++)
    {
        const auto position_to_look = omath::Vector3<float>{0.f, 0.f, dist(gen)};
        if (cam.get_origin().distance_to(position_to_look) < 10)
            continue;

        cam.look_at(position_to_look);

        auto projected_pos = cam.world_to_view_port(position_to_look);

        EXPECT_TRUE(projected_pos.has_value());

        if (!projected_pos)
            continue;

        if (std::abs(projected_pos->x - 0.f) >= 0.01f || std::abs(projected_pos->y - 0.f) >= 0.01f)
            failed_points++;
    }
    EXPECT_LE(failed_points, 100);
}
TEST(unit_test_frostbite_engine, look_at_right)
{
    const auto angles = omath::frostbite_engine::CameraTrait::calc_look_at_angle({}, omath::frostbite_engine::k_abs_right);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = omath::frostbite_engine::forward_vector(angles);
    for (const auto& [result, etalon] :
         std::views::zip(dir_vector.as_array(), omath::frostbite_engine::k_abs_right.as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}
TEST(unit_test_frostbite_engine, look_at_up)
{
    const auto angles = omath::frostbite_engine::CameraTrait::calc_look_at_angle({}, omath::frostbite_engine::k_abs_right);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = omath::frostbite_engine::forward_vector(angles);
    for (const auto& [result, etalon] :
         std::views::zip(dir_vector.as_array(), omath::frostbite_engine::k_abs_right.as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}

TEST(unit_test_frostbite_engine, look_at_back)
{
    const auto angles = omath::frostbite_engine::CameraTrait::calc_look_at_angle({}, -omath::frostbite_engine::k_abs_forward);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = omath::frostbite_engine::forward_vector(angles);
    for (const auto& [result, etalon] :
         std::views::zip(dir_vector.as_array(), (-omath::frostbite_engine::k_abs_forward).as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}
TEST(unit_test_frostbite_engine, look_at_left)
{
    const auto angles = omath::frostbite_engine::CameraTrait::calc_look_at_angle({}, -omath::frostbite_engine::k_abs_right);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = omath::frostbite_engine::forward_vector(angles);
    for (const auto& [result, etalon] :
         std::views::zip(dir_vector.as_array(), (-omath::frostbite_engine::k_abs_right).as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}
TEST(unit_test_frostbite_engine, look_at_down)
{
    const auto angles = omath::frostbite_engine::CameraTrait::calc_look_at_angle({}, -omath::frostbite_engine::k_abs_up);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = omath::frostbite_engine::forward_vector(angles);
    for (const auto& [result, etalon] :
         std::views::zip(dir_vector.as_array(), (-omath::frostbite_engine::k_abs_up).as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}
