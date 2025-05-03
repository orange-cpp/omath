//
// Created by Vlad on 01.09.2024.
//
#include <gtest/gtest.h>
#include <omath/color.hpp>


using namespace omath;

class UnitTestColor : public ::testing::Test
{
protected:
    Color color1;
    Color color2;

    void SetUp() override
    {
        color1 = Color::red();
        color2 = Color::green();
    }
};

// Test constructors
TEST_F(UnitTestColor, Constructor_Float)
{
    constexpr Color color(0.5f, 0.5f, 0.5f, 1.0f);
    EXPECT_FLOAT_EQ(color.x, 0.5f);
    EXPECT_FLOAT_EQ(color.y, 0.5f);
    EXPECT_FLOAT_EQ(color.z, 0.5f);
    EXPECT_FLOAT_EQ(color.w, 1.0f);
}

TEST_F(UnitTestColor, Constructor_Vector4)
{
    constexpr omath::Vector4 vec(0.2f, 0.4f, 0.6f, 0.8f);
    Color color(vec);
    EXPECT_FLOAT_EQ(color.x, 0.2f);
    EXPECT_FLOAT_EQ(color.y, 0.4f);
    EXPECT_FLOAT_EQ(color.z, 0.6f);
    EXPECT_FLOAT_EQ(color.w, 0.8f);
}

// Test static methods for color creation
TEST_F(UnitTestColor, FromRGBA)
{
    constexpr Color color = Color::from_rgba(128, 64, 32, 255);
    EXPECT_FLOAT_EQ(color.x, 128.0f / 255.0f);
    EXPECT_FLOAT_EQ(color.y, 64.0f / 255.0f);
    EXPECT_FLOAT_EQ(color.z, 32.0f / 255.0f);
    EXPECT_FLOAT_EQ(color.w, 1.0f);
}

TEST_F(UnitTestColor, FromHSV)
{
    constexpr Color color = Color::from_hsv(0.0f, 1.0f, 1.0f); // Red in HSV
    EXPECT_FLOAT_EQ(color.x, 1.0f);
    EXPECT_FLOAT_EQ(color.y, 0.0f);
    EXPECT_FLOAT_EQ(color.z, 0.0f);
    EXPECT_FLOAT_EQ(color.w, 1.0f);
}

// Test HSV conversion
TEST_F(UnitTestColor, ToHSV)
{
    Hsv hsv = color1.to_hsv(); // Red color
    EXPECT_FLOAT_EQ(hsv.hue, 0.0f);
    EXPECT_FLOAT_EQ(hsv.saturation, 1.0f);
    EXPECT_FLOAT_EQ(hsv.value, 1.0f);
}

// Test color blending
TEST_F(UnitTestColor, Blend)
{
    Color blended = color1.blend(color2, 0.5f);
    EXPECT_FLOAT_EQ(blended.x, 0.5f);
    EXPECT_FLOAT_EQ(blended.y, 0.5f);
    EXPECT_FLOAT_EQ(blended.z, 0.0f);
    EXPECT_FLOAT_EQ(blended.w, 1.0f);
}

// Test predefined colors
TEST_F(UnitTestColor, PredefinedColors)
{
    constexpr Color red = Color::red();
    constexpr Color green = Color::green();
    constexpr Color blue = Color::blue();

    EXPECT_FLOAT_EQ(red.x, 1.0f);
    EXPECT_FLOAT_EQ(red.y, 0.0f);
    EXPECT_FLOAT_EQ(red.z, 0.0f);
    EXPECT_FLOAT_EQ(red.w, 1.0f);

    EXPECT_FLOAT_EQ(green.x, 0.0f);
    EXPECT_FLOAT_EQ(green.y, 1.0f);
    EXPECT_FLOAT_EQ(green.z, 0.0f);
    EXPECT_FLOAT_EQ(green.w, 1.0f);

    EXPECT_FLOAT_EQ(blue.x, 0.0f);
    EXPECT_FLOAT_EQ(blue.y, 0.0f);
    EXPECT_FLOAT_EQ(blue.z, 1.0f);
    EXPECT_FLOAT_EQ(blue.w, 1.0f);
}

// Test non-member function: Blend for Vector3
TEST_F(UnitTestColor, BlendVector3)
{
    constexpr Color v1(1.0f, 0.0f, 0.0f, 1.f); // Red
    constexpr Color v2(0.0f, 1.0f, 0.0f, 1.f); // Green
    constexpr Color blended = v1.blend(v2, 0.5f);
    EXPECT_FLOAT_EQ(blended.x, 0.5f);
    EXPECT_FLOAT_EQ(blended.y, 0.5f);
    EXPECT_FLOAT_EQ(blended.z, 0.0f);
}