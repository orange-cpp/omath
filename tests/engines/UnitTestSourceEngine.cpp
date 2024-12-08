//
// Created by Orange on 11/23/2024.
//
#include <gtest/gtest.h>
#include <omath/engines/Source/Camera.hpp>
#include <omath/engines/Source/Constants.h>
#include <omath/engines/Source/Formulas.hpp>


TEST(UnitTestSourceEngine, ForwardVector)
{
    const auto forward = omath::source::ForwardVector({{}, {}, {}});

    EXPECT_EQ(forward, omath::source::kAbsForward);
}

TEST(UnitTestSourceEngine, RightVector)
{
    const auto right = omath::source::RightVector({});

    EXPECT_EQ(right, omath::source::kAbsRight);
}

TEST(UnitTestSourceEngine, UpVector)
{
    const auto up = omath::source::UpVector({});
    EXPECT_EQ(up, omath::source::kAbsUp);
}

TEST(UnitTestSourceEngine, PerpectiveProjectionAtCenter)
{
    constexpr auto fov = omath::projection::FieldOfView::FromDegrees(90.f);
    auto cam = omath::source::Camera({0, 0, 0}, {}, {1920.f, 1080.f}, fov, 0.01f, 1000.f);


    const auto viewProjMatrix = cam.GetViewProjectionMatrix();

    for (float distance = 0.02f; distance < 1000.f; distance += 0.01f)
    {
        const auto projected = cam.WorldToScreen(viewProjMatrix, {distance, 0, 0});

        EXPECT_TRUE(projected.has_value());

        if (!projected.has_value())
            continue;

        EXPECT_NEAR(projected->x, 960, 0.00001f);
        EXPECT_NEAR(projected->y, 540, 0.00001f);
    }
}