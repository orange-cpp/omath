//
// Created by Orange on 11/30/2024.
//
#include <gtest/gtest.h>
#include <omath/Angles.hpp>


TEST(UnitTestAngles, RadiansToDeg)
{
    constexpr float rad = 67;

    EXPECT_NEAR(omath::angles::RadiansToDegrees(rad), 3838.82f, 0.01f);
}

TEST(UnitTestAngles, DegreesToRadians)
{
    constexpr float degree = 90;

    EXPECT_NEAR(omath::angles::DegreesToRadians(degree), 1.5708f, 0.01f);
}

TEST(UnitTestAngles, HorizontalFovToVerical)
{
    constexpr float hFov = 90;
    constexpr float aspectRation = 16.0f / 9.0f;
    const auto verticalFov = omath::angles::HorizontalFovToVertical(hFov, aspectRation);

    EXPECT_NEAR(verticalFov, 58.71f, 0.01f);
}

TEST(UnitTestAngles, VerticalToHorizontal)
{
    constexpr float vFov = 58.71;
    constexpr float aspectRation = 16.0f / 9.0f;
    const auto horizontalFov = omath::angles::VerticalFovToHorizontal(vFov, aspectRation);

    EXPECT_NEAR(horizontalFov, 89.99f, 0.01f);
}