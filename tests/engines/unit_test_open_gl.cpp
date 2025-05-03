//
// Created by Orange on 11/23/2024.
//
#include <gtest/gtest.h>
#include <omath/engines/opengl_engine/camera.hpp>
#include <omath/engines/opengl_engine/constants.hpp>
#include <omath/engines/opengl_engine/formulas.hpp>


TEST(UnitTestOpenGL, ForwardVector)
{
    const auto forward = omath::opengl_engine::forward_vector({});
    EXPECT_EQ(forward, omath::opengl_engine::k_abs_forward);
}

TEST(UnitTestOpenGL, RightVector)
{
    const auto right = omath::opengl_engine::right_vector({});
    EXPECT_EQ(right, omath::opengl_engine::k_abs_right);
}

TEST(UnitTestOpenGL, UpVector)
{
    const auto up = omath::opengl_engine::up_vector({});
    EXPECT_EQ(up, omath::opengl_engine::k_abs_up);
}

TEST(UnitTestOpenGL, ForwardVectorRotationYaw)
{
    omath::opengl_engine::ViewAngles angles;

    angles.yaw = omath::opengl_engine::YawAngle::from_degrees(90.f);

    const auto forward = omath::opengl_engine::forward_vector(angles);
    EXPECT_NEAR(forward.x, omath::opengl_engine::k_abs_right.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::opengl_engine::k_abs_right.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::opengl_engine::k_abs_right.z, 0.00001f);
}



TEST(UnitTestOpenGL, ForwardVectorRotationPitch)
{
    omath::opengl_engine::ViewAngles angles;

    angles.pitch = omath::opengl_engine::PitchAngle::from_degrees(-90.f);

    const auto forward = omath::opengl_engine::forward_vector(angles);
    EXPECT_NEAR(forward.x, omath::opengl_engine::k_abs_up.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::opengl_engine::k_abs_up.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::opengl_engine::k_abs_up.z, 0.00001f);
}

TEST(UnitTestOpenGL, ForwardVectorRotationRoll)
{
    omath::opengl_engine::ViewAngles angles;

    angles.roll = omath::opengl_engine::RollAngle::from_degrees(-90.f);

    const auto forward = omath::opengl_engine::up_vector(angles);
    EXPECT_NEAR(forward.x, omath::opengl_engine::k_abs_right.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::opengl_engine::k_abs_right.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::opengl_engine::k_abs_right.z, 0.00001f);
}

TEST(UnitTestOpenGL, ProjectTargetMovedFromCamera)
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

TEST(UnitTestOpenGL, CameraSetAndGetFov)
{
    constexpr auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 90.f);
    cam.set_field_of_view(omath::projection::FieldOfView::from_degrees(50.f));

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 50.f);
}

TEST(UnitTestOpenGL, CameraSetAndGetOrigin)
{
    auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, {}, 0.01f, 1000.f);

    EXPECT_EQ(cam.get_origin(), omath::Vector3<float>{});
    cam.set_field_of_view(omath::projection::FieldOfView::from_degrees(50.f));

    EXPECT_EQ(cam.get_field_of_view().as_degrees(), 50.f);
}