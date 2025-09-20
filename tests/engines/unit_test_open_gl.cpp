//
// Created by Orange on 11/23/2024.
//
#include <gtest/gtest.h>
#include <omath/engines/opengl_engine/camera.hpp>
#include <omath/engines/opengl_engine/constants.hpp>
#include <omath/engines/opengl_engine/formulas.hpp>
#include <random>

TEST(unit_test_opengl, ForwardVector)
{
    const auto forward = omath::opengl_engine::forward_vector({});
    EXPECT_EQ(forward, omath::opengl_engine::k_abs_forward);
}

TEST(unit_test_opengl, RightVector)
{
    const auto right = omath::opengl_engine::right_vector({});
    EXPECT_EQ(right, omath::opengl_engine::k_abs_right);
}

TEST(unit_test_opengl, UpVector)
{
    const auto up = omath::opengl_engine::up_vector({});
    EXPECT_EQ(up, omath::opengl_engine::k_abs_up);
}

TEST(unit_test_opengl, ForwardVectorRotationYaw)
{
    omath::opengl_engine::ViewAngles angles;

    angles.yaw = omath::opengl_engine::YawAngle::from_degrees(-90.f);

    const auto forward = omath::opengl_engine::forward_vector(angles);
    EXPECT_NEAR(forward.x, omath::opengl_engine::k_abs_right.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::opengl_engine::k_abs_right.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::opengl_engine::k_abs_right.z, 0.00001f);
}



TEST(unit_test_opengl, ForwardVectorRotationPitch)
{
    omath::opengl_engine::ViewAngles angles;

    angles.pitch = omath::opengl_engine::PitchAngle::from_degrees(90.f);

    const auto forward = omath::opengl_engine::forward_vector(angles);
    EXPECT_NEAR(forward.x, omath::opengl_engine::k_abs_up.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::opengl_engine::k_abs_up.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::opengl_engine::k_abs_up.z, 0.00001f);
}

TEST(unit_test_opengl, ForwardVectorRotationRoll)
{
    omath::opengl_engine::ViewAngles angles;

    angles.roll = omath::opengl_engine::RollAngle::from_degrees(-90.f);

    const auto forward = omath::opengl_engine::up_vector(angles);
    EXPECT_NEAR(forward.x, omath::opengl_engine::k_abs_right.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::opengl_engine::k_abs_right.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::opengl_engine::k_abs_right.z, 0.00001f);
}

TEST(unit_test_opengl, ProjectTargetMovedFromCamera)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    const auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);


    for (float distance = -10.f; distance > -1000.f; distance -= 0.01f)
    {
        const auto projected = cam.world_to_screen({0, 0, distance});

        EXPECT_TRUE(projected.has_value());

        if (!projected.has_value())
            continue;

        EXPECT_NEAR(projected->x, 960, 0.00001f);
        EXPECT_NEAR(projected->y, 540, 0.00001f);
    }
}

TEST(unit_test_opengl, CameraSetAndGetFov)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 90.f);
    cam.set_field_of_view(omath::projection::FieldOfView::from_degrees(50.f));

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 50.f);
}

TEST(unit_test_opengl, CameraSetAndGetOrigin)
{
    auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, {}, 0.01f, 1000.f);

    EXPECT_EQ(cam.get_origin(), omath::Vector3<float>{});
    cam.set_field_of_view(omath::projection::FieldOfView::from_degrees(50.f));

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 50.f);
}
TEST(unit_test_opengl_engine, loook_at_random_all_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);



    for (int i = 0; i < 100; i++)
    {
        const auto position_to_look = omath::Vector3<float>{dist(gen), dist(gen), dist(gen)};

        if (cam.get_origin().distance_to(position_to_look) < 10)
            continue;
        cam.look_at(position_to_look);

        auto projected_pos = cam.world_to_view_port(position_to_look);

        EXPECT_TRUE(projected_pos.has_value());

        if (!projected_pos)
            continue;

        EXPECT_NEAR(projected_pos->x, 0.f, 0.01f);
        EXPECT_NEAR(projected_pos->y, 0.f, 0.01f);
    }
}

TEST(unit_test_opengl_engine, loook_at_random_x_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-500.f, 500.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);



    for (int i = 0; i < 1000; i++)
    {
        const auto position_to_look = omath::Vector3<float>{dist(gen), 0.f, 0.f};
        cam.look_at(position_to_look);

        auto projected_pos = cam.world_to_view_port(position_to_look);

        EXPECT_TRUE(projected_pos.has_value());

        if (!projected_pos)
            continue;

        EXPECT_NEAR(projected_pos->x, 0.f, 0.00001f);
        EXPECT_NEAR(projected_pos->y, 0.f, 0.00001f);
    }
}

TEST(unit_test_opengl_engine, loook_at_random_y_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-500.f, 500.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);



    for (int i = 0; i < 1000; i++)
    {
        const auto position_to_look = omath::Vector3<float>{0.f, dist(gen), 0.f};
        cam.look_at(position_to_look);

        auto projected_pos = cam.world_to_view_port(position_to_look);

        EXPECT_TRUE(projected_pos.has_value());

        if (!projected_pos)
            continue;

        EXPECT_NEAR(projected_pos->x, 0.f, 0.00001f);
        EXPECT_NEAR(projected_pos->y, 0.f, 0.00001f);
    }
}

TEST(unit_test_opengl_engine, loook_at_random_z_axis)
{
    std::mt19937 gen(std::random_device{}()); // Seed with a non-deterministic source
    std::uniform_real_distribution<float> dist(-500.f, 500.f);

    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.001f, 10000.f);



    for (int i = 0; i < 1000; i++)
    {
        const auto position_to_look = omath::Vector3<float>{0.f, 0.f, dist(gen)};
        cam.look_at(position_to_look);

        auto projected_pos = cam.world_to_view_port(position_to_look);

        EXPECT_TRUE(projected_pos.has_value());

        if (!projected_pos)
            continue;

        EXPECT_NEAR(projected_pos->x, 0.f, 0.00001f);
        EXPECT_NEAR(projected_pos->y, 0.f, 0.025f);
    }
}