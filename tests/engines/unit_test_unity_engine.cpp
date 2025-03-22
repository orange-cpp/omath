//
// Created by Orange on 11/27/2024.
//
//
// Created by Orange on 11/23/2024.
//
#include <gtest/gtest.h>
#include <omath/engines/unity_engine/camera.hpp>
#include <omath/engines/unity_engine/constants.hpp>
#include <omath/engines/unity_engine/formulas.hpp>


TEST(UnitTestUnityEngine, ForwardVector)
{
    const auto forward = omath::unity_engine::ForwardVector({});

    EXPECT_EQ(forward, omath::unity_engine::kAbsForward);
}

TEST(UnitTestUnityEngine, RightVector)
{
    const auto right = omath::unity_engine::RightVector({});

    EXPECT_EQ(right, omath::unity_engine::kAbsRight);
}

TEST(UnitTestUnityEngine, UpVector)
{
    const auto up = omath::unity_engine::UpVector({});
    EXPECT_EQ(up, omath::unity_engine::kAbsUp);
}

/*TEST(UnitTestUnityEngine, ProjectTargetMovedFromCamera)
{
    constexpr auto fov = omath::projection::FieldOfView::FromDegrees(60.f);
    const auto cam = omath::unity_engine::Camera({0, 0, 0}, {}, {1280.f, 720.f}, fov, 0.01f, 1000.f);


    for (float distance = 0.02f; distance < 100.f; distance += 0.01f)
    {
        const auto projected = cam.WorldToScreen({0, 0, distance});

        EXPECT_TRUE(projected.has_value());

        if (!projected.has_value())
            continue;

        EXPECT_NEAR(projected->x, 640, 0.00001f);
        EXPECT_NEAR(projected->y, 360, 0.00001f);
    }
}*/
TEST(UnitTestUnityEngine, Project)
{
    constexpr auto fov = omath::projection::FieldOfView::FromDegrees(60.f);

    const auto cam = omath::unity_engine::Camera({0, 0, 0}, {}, {1280.f, 720.f}, fov, 0.03f, 1000.f);
    const auto proj = cam.WorldToScreen({0.f, 2.f, 10.f});
}

TEST(UnitTestUnityEngine, CameraSetAndGetFov)
{
    constexpr auto fov = omath::projection::FieldOfView::FromDegrees(90.f);
    auto cam = omath::unity_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    EXPECT_EQ(cam.GetFieldOfView().AsDegrees(), 90.f);
    cam.SetFieldOfView(omath::projection::FieldOfView::FromDegrees(50.f));

    EXPECT_EQ(cam.GetFieldOfView().AsDegrees(), 50.f);
}

TEST(UnitTestUnityEngine, CameraSetAndGetOrigin)
{
    auto cam = omath::unity_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, {}, 0.01f, 1000.f);

    EXPECT_EQ(cam.GetOrigin(), omath::Vector3<float>{});
    cam.SetFieldOfView(omath::projection::FieldOfView::FromDegrees(50.f));

    EXPECT_EQ(cam.GetFieldOfView().AsDegrees(), 50.f);
}