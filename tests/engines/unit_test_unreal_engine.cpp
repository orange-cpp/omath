//
// Created by Orange on 11/27/2024.
//
#include <gtest/gtest.h>
#include <omath/engines/unreal_engine/camera.hpp>
#include <omath/engines/unreal_engine/constants.hpp>
#include <omath/engines/unreal_engine/formulas.hpp>
#include <print>

TEST(unit_test_unreal_engine, ForwardVector)
{
    const auto forward = omath::unreal_engine::forward_vector({});

    EXPECT_EQ(forward, omath::unreal_engine::k_abs_forward);
}

TEST(unit_test_unreal_engine, ForwardVectorRotationYaw)
{
    omath::unreal_engine::ViewAngles angles;

    angles.yaw = omath::unreal_engine::YawAngle::from_degrees(90.f);

    const auto forward = omath::unreal_engine::forward_vector(angles);
    EXPECT_NEAR(forward.x, omath::unreal_engine::k_abs_right.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::unreal_engine::k_abs_right.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::unreal_engine::k_abs_right.z, 0.00001f);
}

TEST(unit_test_unreal_engine, ForwardVectorRotationPitch)
{
    omath::unreal_engine::ViewAngles angles;

    angles.pitch = omath::unreal_engine::PitchAngle::from_degrees(-90.f);

    const auto forward = omath::unreal_engine::forward_vector(angles);
    EXPECT_NEAR(forward.x, omath::unreal_engine::k_abs_up.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::unreal_engine::k_abs_up.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::unreal_engine::k_abs_up.z, 0.00001f);
}

TEST(unit_test_unreal_engine, ForwardVectorRotationRoll)
{
    omath::unreal_engine::ViewAngles angles;

    angles.roll = omath::unreal_engine::RollAngle::from_degrees(-90.f);

    const auto forward = omath::unreal_engine::up_vector(angles);
    EXPECT_NEAR(forward.x, omath::unreal_engine::k_abs_right.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::unreal_engine::k_abs_right.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::unreal_engine::k_abs_right.z, 0.00001f);
}

TEST(unit_test_unreal_engine, RightVector)
{
    const auto right = omath::unreal_engine::right_vector({});

    EXPECT_EQ(right, omath::unreal_engine::k_abs_right);
}

TEST(unit_test_unreal_engine, UpVector)
{
    const auto up = omath::unreal_engine::up_vector({});
    EXPECT_EQ(up, omath::unreal_engine::k_abs_up);
}

TEST(unit_test_unreal_engine, ProjectTargetMovedFromCamera)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(60.f);
    const auto cam = omath::unreal_engine::Camera({0, 0, 0}, {}, {1280.f, 720.f}, fov, 0.01f, 1000.f);


    for (float distance = 0.02f; distance < 100.f; distance += 0.01f)
    {
        const auto projected = cam.world_to_screen({distance, 0, 0});

        EXPECT_TRUE(projected.has_value());

        if (!projected.has_value())
            continue;

        EXPECT_NEAR(projected->x, 640, 0.00001f);
        EXPECT_NEAR(projected->y, 360, 0.00001f);
    }
}

TEST(unit_test_unreal_engine, CameraSetAndGetFov)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::unreal_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 90.f);
    cam.set_field_of_view(omath::projection::FieldOfView::from_degrees(50.f));

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 50.f);
}

TEST(unit_test_unreal_engine, CameraSetAndGetOrigin)
{
    auto cam = omath::unreal_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, {}, 0.01f, 1000.f);

    EXPECT_EQ(cam.get_origin(), omath::Vector3<float>{});
    cam.set_field_of_view(omath::projection::FieldOfView::from_degrees(50.f));

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 50.f);
}