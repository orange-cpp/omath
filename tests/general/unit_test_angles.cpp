//
// Created by Orange on 11/30/2024.
//
#include <gtest/gtest.h>
#include <omath/angles.hpp>

TEST(unit_test_angles, radians_to_deg)
{
    constexpr float rad = 67;

    EXPECT_NEAR(omath::angles::radians_to_degrees(rad), 3838.82f, 0.01f);
}

TEST(unit_test_angles, degrees_to_radians)
{
    constexpr float degree = 90;

    EXPECT_NEAR(omath::angles::degrees_to_radians(degree), 1.5708f, 0.01f);
}

TEST(unit_test_angles, horizontal_fov_to_verical)
{
    constexpr float hFov = 90;
    constexpr float aspectRation = 16.0f / 9.0f;
    const auto verticalFov = omath::angles::horizontal_fov_to_vertical(hFov, aspectRation);

    EXPECT_NEAR(verticalFov, 58.71f, 0.01f);
}

TEST(unit_test_angles, vertical_to_horizontal)
{
    constexpr float vFov = 58.71;
    constexpr float aspectRation = 16.0f / 9.0f;
    const auto horizontalFov = omath::angles::vertical_fov_to_horizontal(vFov, aspectRation);

    EXPECT_NEAR(horizontalFov, 89.99f, 0.01f);
}
TEST(unit_test_angles, wrap_angle)
{
    const float wrapped = omath::angles::wrap_angle(361.f, 0.f, 360.f);

    EXPECT_NEAR(wrapped, 1.f, 0.01f);
}
TEST(unit_test_angles, wrap_angle_negative_range)
{
    const float wrapped = omath::angles::wrap_angle(-90.f, 0.f, 360.f);

    EXPECT_NEAR(wrapped, 270.f, 0.01f);
}