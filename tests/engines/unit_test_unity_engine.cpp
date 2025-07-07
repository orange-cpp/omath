//
// Created by Orange on 11/27/2024.
//
#include <gtest/gtest.h>
#include <omath/engines/unity_engine/camera.hpp>
#include <omath/engines/unity_engine/constants.hpp>
#include <omath/engines/unity_engine/formulas.hpp>
#include <print>

TEST(unit_test_unity_engine, ForwardVector)
{
    const auto forward = omath::unity_engine::forward_vector({});

    EXPECT_EQ(forward, omath::unity_engine::k_abs_forward);
}

TEST(unit_test_unity_engine, ForwardVectorRotationYaw)
{
    omath::unity_engine::ViewAngles angles;

    angles.yaw = omath::unity_engine::YawAngle::from_degrees(90.f);

    const auto forward = omath::unity_engine::forward_vector(angles);
    EXPECT_NEAR(forward.x, omath::unity_engine::k_abs_right.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::unity_engine::k_abs_right.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::unity_engine::k_abs_right.z, 0.00001f);
}

TEST(unit_test_unity_engine, ForwardVectorRotationPitch)
{
    omath::unity_engine::ViewAngles angles;

    angles.pitch = omath::unity_engine::PitchAngle::from_degrees(-90.f);

    const auto forward = omath::unity_engine::forward_vector(angles);
    EXPECT_NEAR(forward.x, omath::unity_engine::k_abs_up.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::unity_engine::k_abs_up.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::unity_engine::k_abs_up.z, 0.00001f);
}

TEST(unit_test_unity_engine, ForwardVectorRotationRoll)
{
    omath::unity_engine::ViewAngles angles;

    angles.roll = omath::unity_engine::RollAngle::from_degrees(-90.f);

    const auto forward = omath::unity_engine::up_vector(angles);
    EXPECT_NEAR(forward.x, omath::unity_engine::k_abs_right.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::unity_engine::k_abs_right.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::unity_engine::k_abs_right.z, 0.00001f);
}

TEST(unit_test_unity_engine, RightVector)
{
    const auto right = omath::unity_engine::right_vector({});

    EXPECT_EQ(right, omath::unity_engine::k_abs_right);
}

TEST(unit_test_unity_engine, UpVector)
{
    const auto up = omath::unity_engine::up_vector({});
    EXPECT_EQ(up, omath::unity_engine::k_abs_up);
}

TEST(unit_test_unity_engine, ProjectTargetMovedFromCamera)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(60.f);
    const auto cam = omath::unity_engine::Camera({0, 0, 0}, {}, {1280.f, 720.f}, fov, 0.01f, 1000.f);


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
TEST(unit_test_unity_engine, Project)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(60.f);

    const auto cam = omath::unity_engine::Camera({0, 0, 0}, {}, {1280.f, 720.f}, fov, 0.03f, 1000.f);
    const auto proj = cam.world_to_screen({5.f, 3, 10.f});
    std::println("{} {}", proj->x, proj->y);
}

TEST(unit_test_unity_engine, CameraSetAndGetFov)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::unity_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 90.f);
    cam.set_field_of_view(omath::projection::FieldOfView::from_degrees(50.f));

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 50.f);
}

TEST(unit_test_unity_engine, CameraSetAndGetOrigin)
{
    auto cam = omath::unity_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, {}, 0.01f, 1000.f);

    EXPECT_EQ(cam.get_origin(), omath::Vector3<float>{});
    cam.set_field_of_view(omath::projection::FieldOfView::from_degrees(50.f));

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 50.f);
}