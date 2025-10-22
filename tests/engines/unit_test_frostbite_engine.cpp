//
// Created by Vlad on 10/23/2025.
//
#include <gtest/gtest.h>
#include <omath/engines/frostbite_engine/camera.hpp>
#include <omath/engines/frostbite_engine/constants.hpp>
#include <omath/engines/frostbite_engine/formulas.hpp>
#include <print>
#include <random>

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