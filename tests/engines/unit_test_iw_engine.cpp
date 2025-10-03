//
// Created by Vlad on 3/17/2025.
//
#include <gtest/gtest.h>
import omath.iw_engine.constants;
import omath.iw_engine.formulas;
import omath.iw_engine.camera;


#include <random>

TEST(unit_test_iw_engine, ForwardVector)
{
    const auto forward = omath::iw_engine::forward_vector({});

    EXPECT_EQ(forward, omath::iw_engine::k_abs_forward);
}

TEST(unit_test_iw_engine, RightVector)
{
    const auto right = omath::iw_engine::right_vector({});

    EXPECT_EQ(right, omath::iw_engine::k_abs_right);
}

TEST(unit_test_iw_engine, UpVector)
{
    const auto up = omath::iw_engine::up_vector({});
    EXPECT_EQ(up, omath::iw_engine::k_abs_up);
}

TEST(unit_test_iw_engine, ForwardVectorRotationYaw)
{
    omath::iw_engine::ViewAngles angles;

    angles.yaw = omath::iw_engine::YawAngle::from_degrees(-90.f);

    const auto forward = omath::iw_engine::forward_vector(angles);
    EXPECT_NEAR(forward.x, omath::iw_engine::k_abs_right.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::iw_engine::k_abs_right.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::iw_engine::k_abs_right.z, 0.00001f);
}

TEST(unit_test_iw_engine, ForwardVectorRotationPitch)
{
    omath::iw_engine::ViewAngles angles;

    angles.pitch = omath::iw_engine::PitchAngle::from_degrees(-89.f);

    const auto forward = omath::iw_engine::forward_vector(angles);
    EXPECT_NEAR(forward.x, omath::iw_engine::k_abs_up.x, 0.02f);
    EXPECT_NEAR(forward.y, omath::iw_engine::k_abs_up.y, 0.01f);
    EXPECT_NEAR(forward.z, omath::iw_engine::k_abs_up.z, 0.01f);
}

TEST(unit_test_iw_engine, ForwardVectorRotationRoll)
{
    omath::iw_engine::ViewAngles angles;

    angles.roll = omath::iw_engine::RollAngle::from_degrees(90.f);

    const auto forward = omath::iw_engine::up_vector(angles);
    EXPECT_NEAR(forward.x, omath::iw_engine::k_abs_right.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::iw_engine::k_abs_right.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::iw_engine::k_abs_right.z, 0.00001f);
}

TEST(unit_test_iw_engine, ProjectTargetMovedFromCamera)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::iw_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

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

TEST(unit_test_iw_engine, CameraSetAndGetFov)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::iw_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 90.f);
    cam.set_field_of_view(omath::projection::FieldOfView::from_degrees(50.f));

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 50.f);
}

TEST(unit_test_iw_engine, CameraSetAndGetOrigin)
{
    auto cam = omath::iw_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, {}, 0.01f, 1000.f);

    EXPECT_EQ(cam.get_origin(), omath::Vector3<float>{});
    cam.set_field_of_view(omath::projection::FieldOfView::from_degrees(50.f));

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 50.f);
}

TEST(unit_test_iw_engine, loook_at_random_all_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::iw_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);

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

        if (std::abs(projected_pos->x - 0.f) >= 0.001f || std::abs(projected_pos->y - 0.f) >= 0.001f)
            failed_points++;
    }
    EXPECT_LE(failed_points, 100);
}

TEST(unit_test_iw_engine, loook_at_random_x_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::iw_engine::Camera({dist(gen), dist(gen), dist(gen)}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);

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

TEST(unit_test_iw_engine, loook_at_random_y_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::iw_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);

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

TEST(unit_test_iw_engine, loook_at_random_z_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::iw_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);

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