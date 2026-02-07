//
// Created by Orange on 11/23/2024.
//
#include <gtest/gtest.h>
#include <omath/engines/source_engine/camera.hpp>
#include <omath/engines/source_engine/constants.hpp>
#include <omath/engines/source_engine/formulas.hpp>
#include <random>

TEST(unit_test_source_engine_units, HammerUnitsToCentimeters_BasicValues)
{
    EXPECT_FLOAT_EQ(omath::source_engine::hammer_units_to_centimeters(0.0f), 0.0f);
    EXPECT_FLOAT_EQ(omath::source_engine::hammer_units_to_centimeters(1.0f), 2.54f);
    EXPECT_FLOAT_EQ(omath::source_engine::hammer_units_to_centimeters(10.0f), 25.4f);
    EXPECT_FLOAT_EQ(omath::source_engine::hammer_units_to_centimeters(-2.0f), -5.08f);
}

TEST(unit_test_source_engine_units, HammerUnitsToMeters_BasicValues)
{
    EXPECT_NEAR(omath::source_engine::hammer_units_to_meters(0.0), 0.0, 1e-12);
    EXPECT_NEAR(omath::source_engine::hammer_units_to_meters(1.0), 0.0254, 1e-12);
    EXPECT_NEAR(omath::source_engine::hammer_units_to_meters(100.0), 2.54, 1e-12);
    EXPECT_NEAR(omath::source_engine::hammer_units_to_meters(-4.0), -0.1016, 1e-12);
}

TEST(unit_test_source_engine_units, HammerUnitsToKilometers_BasicValues)
{
    EXPECT_NEAR(omath::source_engine::hammer_units_to_kilometers(0.0), 0.0, 1e-15);
    EXPECT_NEAR(omath::source_engine::hammer_units_to_kilometers(1.0), 0.0000254, 1e-15);
    EXPECT_NEAR(omath::source_engine::hammer_units_to_kilometers(1000.0), 0.0254, 1e-15);
    EXPECT_NEAR(omath::source_engine::hammer_units_to_kilometers(-10.0), -0.000254, 1e-15);
}

TEST(unit_test_source_engine_units, CentimetersToHammerUnits_BasicValues)
{
    EXPECT_FLOAT_EQ(omath::source_engine::centimeters_to_hammer_units(0.0f), 0.0f);
    EXPECT_NEAR(omath::source_engine::centimeters_to_hammer_units(2.54f), 1.0f, 1e-6f);
    EXPECT_NEAR(omath::source_engine::centimeters_to_hammer_units(25.4f), 10.0f, 1e-5f);
    EXPECT_NEAR(omath::source_engine::centimeters_to_hammer_units(-5.08f), -2.0f, 1e-6f);
}

TEST(unit_test_source_engine_units, MetersToHammerUnits_BasicValues)
{
    EXPECT_NEAR(omath::source_engine::meters_to_hammer_units(0.0), 0.0, 1e-12);
    EXPECT_NEAR(omath::source_engine::meters_to_hammer_units(0.0254), 1.0, 1e-12);
    EXPECT_NEAR(omath::source_engine::meters_to_hammer_units(2.54), 100.0, 1e-10);
    EXPECT_NEAR(omath::source_engine::meters_to_hammer_units(-0.0508), -2.0, 1e-12);
}

TEST(unit_test_source_engine_units, KilometersToHammerUnits_BasicValues)
{
    EXPECT_NEAR(omath::source_engine::kilometers_to_hammer_units(0.0), 0.0, 1e-9);
    EXPECT_NEAR(omath::source_engine::kilometers_to_hammer_units(0.0000254), 1.0, 1e-9);
    EXPECT_NEAR(omath::source_engine::kilometers_to_hammer_units(0.00254), 100.0, 1e-7);
    EXPECT_NEAR(omath::source_engine::kilometers_to_hammer_units(-0.0000508), -2.0, 1e-9);
}

TEST(unit_test_source_engine_units, RoundTrip_HammerToCentimetersToHammer)
{
    constexpr float hu_f = 123.456f;
    constexpr auto cm_f = omath::source_engine::hammer_units_to_centimeters(hu_f);
    constexpr auto hu_f_back = omath::source_engine::centimeters_to_hammer_units(cm_f);
    EXPECT_NEAR(hu_f_back, hu_f, 1e-5f);

    constexpr double hu_d = -98765.4321;
    constexpr auto cm_d = omath::source_engine::hammer_units_to_centimeters(hu_d);
    constexpr auto hu_d_back = omath::source_engine::centimeters_to_hammer_units(cm_d);
    EXPECT_NEAR(hu_d_back, hu_d, 1e-10);
}

TEST(unit_test_source_engine_units, RoundTrip_HammerToMetersToHammer)
{
    constexpr float hu_f = 2500.25f;
    constexpr auto m_f = omath::source_engine::hammer_units_to_meters(hu_f);
    constexpr auto hu_f_back = omath::source_engine::meters_to_hammer_units(m_f);
    EXPECT_NEAR(hu_f_back, hu_f, 1e-4f);

    constexpr double hu_d = -42000.125;
    constexpr auto m_d = omath::source_engine::hammer_units_to_meters(hu_d);
    constexpr auto hu_d_back = omath::source_engine::meters_to_hammer_units(m_d);
    EXPECT_NEAR(hu_d_back, hu_d, 1e-10);
}

TEST(unit_test_source_engine_units, RoundTrip_HammerToKilometersToHammer)
{
    constexpr float hu_f = 100000.0f;
    constexpr auto km_f = omath::source_engine::hammer_units_to_kilometers(hu_f);
    constexpr auto hu_f_back = omath::source_engine::kilometers_to_hammer_units(km_f);
    EXPECT_NEAR(hu_f_back, hu_f, 1e-2f); // looser due to float scaling

    constexpr double hu_d = -1234567.89;
    constexpr auto km_d = omath::source_engine::hammer_units_to_kilometers(hu_d);
    constexpr auto hu_d_back = omath::source_engine::kilometers_to_hammer_units(km_d);
    EXPECT_NEAR(hu_d_back, hu_d, 1e-7);
}

TEST(unit_test_source_engine_units, ConversionChainConsistency)
{
    // hu -> cm -> m -> km should match direct helpers
    constexpr auto hu = 54321.123;

    constexpr auto cm = omath::source_engine::hammer_units_to_centimeters(hu);
    constexpr auto m_via_cm = cm / 100.0;
    constexpr auto km_via_cm = m_via_cm / 1000.0;

    constexpr auto m_direct = omath::source_engine::hammer_units_to_meters(hu);
    constexpr auto km_direct = omath::source_engine::hammer_units_to_kilometers(hu);

    EXPECT_NEAR(m_direct, m_via_cm, 1e-12);
    EXPECT_NEAR(km_direct, km_via_cm, 1e-15);
}

TEST(unit_test_source_engine_units, SupportsFloatAndDoubleTypes)
{
    static_assert(std::is_same_v<decltype(omath::source_engine::hammer_units_to_centimeters(1.0f)), float>);
    static_assert(std::is_same_v<decltype(omath::source_engine::hammer_units_to_centimeters(1.0)), double>);
    static_assert(std::is_same_v<decltype(omath::source_engine::meters_to_hammer_units(1.0f)), float>);
    static_assert(std::is_same_v<decltype(omath::source_engine::kilometers_to_hammer_units(1.0)), double>);
}

TEST(unit_test_source_engine_units, ConstexprEvaluation)
{
    constexpr double hu = 10.0;
    constexpr double cm = omath::source_engine::hammer_units_to_centimeters(hu);
    constexpr double m = omath::source_engine::hammer_units_to_meters(hu);
    constexpr double km = omath::source_engine::hammer_units_to_kilometers(hu);

    static_assert(cm == 25.4, "constexpr hu->cm failed");
    static_assert(m == 0.254, "constexpr hu->m failed");
    static_assert(km == 0.000254, "constexpr hu->km failed");
}

TEST(unit_test_source_engine, ForwardVector)
{
    const auto forward = omath::source_engine::forward_vector({});

    EXPECT_EQ(forward, omath::source_engine::k_abs_forward);
}

TEST(unit_test_source_engine, RightVector)
{
    const auto right = omath::source_engine::right_vector({});

    EXPECT_EQ(right, omath::source_engine::k_abs_right);
}

TEST(unit_test_source_engine, UpVector)
{
    const auto up = omath::source_engine::up_vector({});
    EXPECT_EQ(up, omath::source_engine::k_abs_up);
}

TEST(unit_test_source_engine, ForwardVectorRotationYaw)
{
    omath::source_engine::ViewAngles angles;

    angles.yaw = omath::source_engine::YawAngle::from_degrees(-90.f);

    const auto forward = omath::source_engine::forward_vector(angles);
    EXPECT_NEAR(forward.x, omath::source_engine::k_abs_right.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::source_engine::k_abs_right.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::source_engine::k_abs_right.z, 0.00001f);
}

TEST(unit_test_source_engine, ForwardVectorRotationPitch)
{
    omath::source_engine::ViewAngles angles;

    angles.pitch = omath::source_engine::PitchAngle::from_degrees(-89.f);

    const auto forward = omath::source_engine::forward_vector(angles);
    EXPECT_NEAR(forward.x, omath::source_engine::k_abs_up.x, 0.02f);
    EXPECT_NEAR(forward.y, omath::source_engine::k_abs_up.y, 0.01f);
    EXPECT_NEAR(forward.z, omath::source_engine::k_abs_up.z, 0.01f);
}

TEST(unit_test_source_engine, ForwardVectorRotationRoll)
{
    omath::source_engine::ViewAngles angles;

    angles.roll = omath::source_engine::RollAngle::from_degrees(90.f);

    const auto forward = omath::source_engine::up_vector(angles);
    EXPECT_NEAR(forward.x, omath::source_engine::k_abs_right.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::source_engine::k_abs_right.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::source_engine::k_abs_right.z, 0.00001f);
}

TEST(unit_test_source_engine, ProjectTargetMovedFromCamera)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    for (float distance = 0.02f; distance < 1000.f; distance += 0.01f)
    {
        const auto projected = cam.world_to_screen({distance, 0, 0});

        EXPECT_TRUE(projected.has_value());

        if (!projected.has_value())
            continue;

        EXPECT_NEAR(projected->x, 960, 0.00001f);
        EXPECT_NEAR(projected->y, 540, 0.00001f);
    }
}

TEST(unit_test_source_engine, ProjectTargetMovedUp)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    auto prev = 1080.f;
    for (float distance = 0.0f; distance < 10.f; distance += 1.f)
    {
        const auto projected = cam.world_to_screen({100.f, 0, distance});
        EXPECT_TRUE(projected.has_value());

        if (!projected.has_value())
            continue;

        EXPECT_TRUE(projected->y < prev);

        prev = projected->y;
    }
}

TEST(unit_test_source_engine, CameraSetAndGetFov)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::source_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 90.f);
    cam.set_field_of_view(omath::projection::FieldOfView::from_degrees(50.f));

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 50.f);
}

TEST(unit_test_source_engine, CameraSetAndGetOrigin)
{
    auto cam = omath::source_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, {}, 0.01f, 1000.f);

    EXPECT_EQ(cam.get_origin(), omath::Vector3<float>{});
    cam.set_field_of_view(omath::projection::FieldOfView::from_degrees(50.f));

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 50.f);
}

TEST(unit_test_source_engine, loook_at_random_all_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::source_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);

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

TEST(unit_test_source_engine, loook_at_random_x_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::source_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);

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

        if (std::abs(projected_pos->x - 0.f) >= 0.01f || std::abs(projected_pos->y - 0.f) >= 0.01f)
            failed_points++;
    }
    EXPECT_LE(failed_points, 100);
}

TEST(unit_test_source_engine, loook_at_random_y_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::source_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);

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

TEST(unit_test_source_engine, loook_at_random_z_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::source_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);

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

        if (std::abs(projected_pos->x - 0.f) >= 0.01f || std::abs(projected_pos->y - 0.f) >= 0.025f)
            failed_points++;
    }
    EXPECT_LE(failed_points, 100);
}