//
// Created by Orange on 11/30/2024.
//
#include <gtest/gtest.h>
import omath.angle;
TEST(unit_test_angles, radians_to_deg)
{
    constexpr float rad = 67;

    EXPECT_NEAR(omath::radians_to_degrees(rad), 3838.82f, 0.01f);
}

TEST(unit_test_angles, degrees_to_radians)
{
    constexpr float degree = 90;

    EXPECT_NEAR(omath::degrees_to_radians(degree), 1.5708f, 0.01f);
}

TEST(unit_test_angles, horizontal_fov_to_verical)
{
    constexpr float h_fov = 90;
    constexpr float aspect_ration = 16.0f / 9.0f;
    const auto vertical_fov = omath::horizontal_fov_to_vertical(h_fov, aspect_ration);

    EXPECT_NEAR(vertical_fov, 58.71f, 0.01f);
}

TEST(unit_test_angles, vertical_to_horizontal)
{
    constexpr float v_fov = 58.71;
    constexpr float aspect_ration = 16.0f / 9.0f;
    const auto horizontal_fov = omath::vertical_fov_to_horizontal(v_fov, aspect_ration);

    EXPECT_NEAR(horizontal_fov, 89.99f, 0.01f);
}
TEST(unit_test_angles, wrap_angle)
{
    const float wrapped = omath::wrap_angle(361.f, 0.f, 360.f);

    EXPECT_NEAR(wrapped, 1.f, 0.01f);
}
TEST(unit_test_angles, wrap_angle_negative_range)
{
    const float wrapped = omath::wrap_angle(-90.f, 0.f, 360.f);

    EXPECT_NEAR(wrapped, 270.f, 0.01f);
}