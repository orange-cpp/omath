//
// Created by Orange on 11/23/2024.
//
#include <gtest/gtest.h>
#include <omath/engines/source_engine/camera.hpp>
#include <omath/engines/source_engine/constants.hpp>
#include <omath/engines/source_engine/formulas.hpp>


TEST(UnitTestSourceEngine, ForwardVector)
{
    const auto forward = omath::source_engine::ForwardVector({});
    omath::source_engine::ViewAngles angles;
    //angles.pitch = omath::source_engine::PitchAngle::FromDegrees(-90);
    const auto forward2 = omath::source_engine::ForwardVector(angles);

    //std::println("{} {} {}", forward2.x, forward2.y, forward2.z);

    EXPECT_EQ(forward, omath::source_engine::kAbsForward);
}

TEST(UnitTestSourceEngine, RightVector)
{
    const auto right = omath::source_engine::RightVector({});

    EXPECT_EQ(right, omath::source_engine::kAbsRight);
}

TEST(UnitTestSourceEngine, UpVector)
{
    const auto up = omath::source_engine::UpVector({});
    EXPECT_EQ(up, omath::source_engine::kAbsUp);
}

TEST(UnitTestSourceEngine, ProjectTargetMovedFromCamera)
{
    constexpr auto fov = omath::projection::FieldOfView::FromDegrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);


    for (float distance = 0.02f; distance < 1000.f; distance += 0.01f)
    {
        const auto projected = cam.WorldToScreen({distance, 0, 0});

        EXPECT_TRUE(projected.has_value());

        if (!projected.has_value())
            continue;

        EXPECT_NEAR(projected->x, 960, 0.00001f);
        EXPECT_NEAR(projected->y, 540, 0.00001f);
    }
}

TEST(UnitTestSourceEngine, ProjectTargetMovedUp)
{
    constexpr auto fov = omath::projection::FieldOfView::FromDegrees(90.f);
    const auto cam = omath::source_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    auto prev = 1080.f;
    for (float distance = 0.0f; distance < 10.f; distance += 1.f)
    {
        const auto projected = cam.WorldToScreen({100.f, 0, distance});
        EXPECT_TRUE(projected.has_value());

        if (!projected.has_value())
            continue;

        EXPECT_TRUE(projected->y < prev);

        prev = projected->y;
    }
}

TEST(UnitTestSourceEngine, CameraSetAndGetFov)
{
    constexpr auto fov = omath::projection::FieldOfView::FromDegrees(90.f);
    auto cam = omath::source_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    EXPECT_EQ(cam.GetFieldOfView().AsDegrees(), 90.f);
    cam.SetFieldOfView(omath::projection::FieldOfView::FromDegrees(50.f));

    EXPECT_EQ(cam.GetFieldOfView().AsDegrees(), 50.f);
}

TEST(UnitTestSourceEngine, CameraSetAndGetOrigin)
{
    auto cam = omath::source_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, {}, 0.01f, 1000.f);

    EXPECT_EQ(cam.GetOrigin(), omath::Vector3<float>{});
    cam.SetFieldOfView(omath::projection::FieldOfView::FromDegrees(50.f));

    EXPECT_EQ(cam.GetFieldOfView().AsDegrees(), 50.f);
}