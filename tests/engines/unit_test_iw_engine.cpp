//
// Created by Vlad on 3/17/2025.
//
#include <gtest/gtest.h>
#include <omath/engines/iw_engine/Camera.hpp>
#include <omath/engines/iw_engine/Constants.hpp>
#include <omath/engines/iw_engine/Formulas.hpp>


TEST(UnitTestEwEngine, ForwardVector)
{
    const auto forward = omath::source_engine::ForwardVector({});

    EXPECT_EQ(forward, omath::source_engine::kAbsForward);
}

TEST(UnitTestEwEngine, RightVector)
{
    const auto right = omath::source_engine::RightVector({});

    EXPECT_EQ(right, omath::source_engine::kAbsRight);
}

TEST(UnitTestEwEngine, UpVector)
{
    const auto up = omath::source_engine::UpVector({});
    EXPECT_EQ(up, omath::source_engine::kAbsUp);
}

TEST(UnitTestEwEngine, ProjectTargetMovedFromCamera)
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

TEST(UnitTestEwEngine, CameraSetAndGetFov)
{
    constexpr auto fov = omath::projection::FieldOfView::FromDegrees(90.f);
    auto cam = omath::source_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    EXPECT_EQ(cam.GetFieldOfView().AsDegrees(), 90.f);
    cam.SetFieldOfView(omath::projection::FieldOfView::FromDegrees(50.f));

    EXPECT_EQ(cam.GetFieldOfView().AsDegrees(), 50.f);
}

TEST(UnitTestEwEngine, CameraSetAndGetOrigin)
{
    auto cam = omath::source_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, {}, 0.01f, 1000.f);

    EXPECT_EQ(cam.GetOrigin(), omath::Vector3<float>{});
    cam.SetFieldOfView(omath::projection::FieldOfView::FromDegrees(50.f));

    EXPECT_EQ(cam.GetFieldOfView().AsDegrees(), 50.f);
}