//
// Created by Vlad on 3/17/2025.
//
#include <gtest/gtest.h>
#include <omath/engines/iw_engine/camera.hpp>
#include <omath/engines/iw_engine/constants.hpp>
#include <omath/engines/iw_engine/formulas.hpp>


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