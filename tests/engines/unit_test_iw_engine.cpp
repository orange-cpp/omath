//
// Created by Vlad on 3/17/2025.
//
#include <gtest/gtest.h>
#include <omath/engines/iw_engine/camera.hpp>
#include <omath/engines/iw_engine/constants.hpp>
#include <omath/engines/iw_engine/formulas.hpp>


TEST(UnitTestIwEngine, ForwardVector)
{
    const auto forward = omath::iw_engine::ForwardVector({});

    EXPECT_EQ(forward, omath::iw_engine::kAbsForward);
}

TEST(UnitTestIwEngine, RightVector)
{
    const auto right = omath::iw_engine::RightVector({});

    EXPECT_EQ(right, omath::iw_engine::kAbsRight);
}

TEST(UnitTestIwEngine, UpVector)
{
    const auto up = omath::iw_engine::UpVector({});
    EXPECT_EQ(up, omath::iw_engine::kAbsUp);
}

TEST(UnitTestIwEngine, ProjectTargetMovedFromCamera)
{
    constexpr auto fov = omath::projection::FieldOfView::FromDegrees(90.f);
    const auto cam = omath::iw_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);


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

TEST(UnitTestIwEngine, CameraSetAndGetFov)
{
    constexpr auto fov = omath::projection::FieldOfView::FromDegrees(90.f);
    auto cam = omath::iw_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);

    EXPECT_EQ(cam.GetFieldOfView().AsDegrees(), 90.f);
    cam.SetFieldOfView(omath::projection::FieldOfView::FromDegrees(50.f));

    EXPECT_EQ(cam.GetFieldOfView().AsDegrees(), 50.f);
}

TEST(UnitTestIwEngine, CameraSetAndGetOrigin)
{
    auto cam = omath::iw_engine::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, {}, 0.01f, 1000.f);

    EXPECT_EQ(cam.GetOrigin(), omath::Vector3<float>{});
    cam.SetFieldOfView(omath::projection::FieldOfView::FromDegrees(50.f));

    EXPECT_EQ(cam.GetFieldOfView().AsDegrees(), 50.f);
}