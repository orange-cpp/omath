//
// Created by Orange on 11/23/2024.
//
#include <gtest/gtest.h>

import omath.source_engine.formulas;
import omath.source_engine.constants;
import omath.source_engine.camera;
import omath.camera;


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