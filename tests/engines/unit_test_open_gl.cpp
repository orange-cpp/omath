//
// Created by Orange on 11/23/2024.
//
#include <gtest/gtest.h>
#include <omath/engines/opengl_engine/camera.hpp>
#include <omath/engines/opengl_engine/constants.hpp>
#include <omath/engines/opengl_engine/formulas.hpp>


TEST(UnitTestOpenGL, ForwardVector)
{
    const auto forward = omath::opengl_engine::ForwardVector({});
    EXPECT_EQ(forward, omath::opengl_engine::kAbsForward);
}

TEST(UnitTestOpenGL, RightVector)
{
    const auto right = omath::opengl_engine::RightVector({});
    EXPECT_EQ(right, omath::opengl_engine::kAbsRight);
}

TEST(UnitTestOpenGL, UpVector)
{
    const auto up = omath::opengl_engine::UpVector({});
    EXPECT_EQ(up, omath::opengl_engine::kAbsUp);
}

TEST(UnitTestOpenGL, ForwardVectorRotationYaw)
{
    omath::opengl_engine::ViewAngles angles;

    angles.yaw = omath::opengl_engine::YawAngle::FromDegrees(90.f);

    const auto forward = omath::opengl_engine::ForwardVector(angles);
    EXPECT_NEAR(forward.x, omath::opengl_engine::kAbsRight.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::opengl_engine::kAbsRight.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::opengl_engine::kAbsRight.z, 0.00001f);
}



TEST(UnitTestOpenGL, ForwardVectorRotationPitch)
{
    omath::opengl_engine::ViewAngles angles;

    angles.pitch = omath::opengl_engine::PitchAngle::FromDegrees(-90.f);

    const auto forward = omath::opengl_engine::ForwardVector(angles);
    EXPECT_NEAR(forward.x, omath::opengl_engine::kAbsUp.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::opengl_engine::kAbsUp.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::opengl_engine::kAbsUp.z, 0.00001f);
}

TEST(UnitTestOpenGL, ForwardVectorRotationRoll)
{
    omath::opengl_engine::ViewAngles angles;

    angles.roll = omath::opengl_engine::RollAngle::FromDegrees(-90.f);

    const auto forward = omath::opengl_engine::UpVector(angles);
    EXPECT_NEAR(forward.x, omath::opengl_engine::kAbsRight.x, 0.00001f);
    EXPECT_NEAR(forward.y, omath::opengl_engine::kAbsRight.y, 0.00001f);
    EXPECT_NEAR(forward.z, omath::opengl_engine::kAbsRight.z, 0.00001f);
}

TEST(UnitTestOpenGL, ProjectTargetMovedFromCamera)
{
    constexpr auto fov = omath::projection::FieldOfView::FromDegrees(90.f);
    const auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);


    for (float distance = -10.f; distance > -1000.f; distance -= 0.01f)
    {
        const auto projected = cam.WorldToScreen({0, 0, distance});

        EXPECT_TRUE(projected.has_value());

        if (!projected.has_value())
            continue;

        EXPECT_NEAR(projected->x, 960, 0.00001f);
        EXPECT_NEAR(projected->y, 540, 0.00001f);
    }
}

TEST(UnitTestOpenGL, CameraSetAndGetFov)
{
    constexpr auto fov = omath::projection::FieldOfView::FromDegrees(90.f);
    auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    EXPECT_EQ(cam.GetFieldOfView().AsDegrees(), 90.f);
    cam.SetFieldOfView(omath::projection::FieldOfView::FromDegrees(50.f));

    EXPECT_EQ(cam.GetFieldOfView().AsDegrees(), 50.f);
}

TEST(UnitTestOpenGL, CameraSetAndGetOrigin)
{
    auto cam = omath::opengl_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, {}, 0.01f, 1000.f);

    EXPECT_EQ(cam.GetOrigin(), omath::Vector3<float>{});
    cam.SetFieldOfView(omath::projection::FieldOfView::FromDegrees(50.f));

    EXPECT_EQ(cam.GetFieldOfView().AsDegrees(), 50.f);
}