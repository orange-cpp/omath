//
// Created by Vlad on 01.09.2024.
//
#include <gtest/gtest.h>
#include <omath/Color.h>


using namespace omath::color;

class ColorTest : public ::testing::Test
{
protected:
    Color color1;
    Color color2;

    void SetUp() override
    {
        color1 = Color::Red();
        color2 = Color::Green();
    }
};

// Test constructors
TEST_F(ColorTest, Constructor_Float)
{
    Color color(0.5f, 0.5f, 0.5f, 1.0f);
    EXPECT_FLOAT_EQ(color.x, 0.5f);
    EXPECT_FLOAT_EQ(color.y, 0.5f);
    EXPECT_FLOAT_EQ(color.z, 0.5f);
    EXPECT_FLOAT_EQ(color.w, 1.0f);
}

TEST_F(ColorTest, Constructor_Vector4)
{
    omath::Vector4 vec(0.2f, 0.4f, 0.6f, 0.8f);
    Color color(vec);
    EXPECT_FLOAT_EQ(color.x, 0.2f);
    EXPECT_FLOAT_EQ(color.y, 0.4f);
    EXPECT_FLOAT_EQ(color.z, 0.6f);
    EXPECT_FLOAT_EQ(color.w, 0.8f);
}

// Test static methods for color creation
TEST_F(ColorTest, FromRGBA)
{
    Color color = Color::FromRGBA(128, 64, 32, 255);
    EXPECT_FLOAT_EQ(color.x, 128.0f / 255.0f);
    EXPECT_FLOAT_EQ(color.y, 64.0f / 255.0f);
    EXPECT_FLOAT_EQ(color.z, 32.0f / 255.0f);
    EXPECT_FLOAT_EQ(color.w, 1.0f);
}

TEST_F(ColorTest, FromHSV)
{
    Color color = Color::FromHSV(0.0f, 1.0f, 1.0f); // Red in HSV
    EXPECT_FLOAT_EQ(color.x, 1.0f);
    EXPECT_FLOAT_EQ(color.y, 0.0f);
    EXPECT_FLOAT_EQ(color.z, 0.0f);
    EXPECT_FLOAT_EQ(color.w, 1.0f);
}

// Test HSV conversion
TEST_F(ColorTest, ToHSV)
{
    HSV hsv = color1.ToHSV(); // Red color
    EXPECT_FLOAT_EQ(hsv.m_hue, 0.0f);
    EXPECT_FLOAT_EQ(hsv.m_saturation, 1.0f);
    EXPECT_FLOAT_EQ(hsv.m_value, 1.0f);
}

// Test color blending
TEST_F(ColorTest, Blend)
{
    Color blended = color1.Blend(color2, 0.5f);
    EXPECT_FLOAT_EQ(blended.x, 0.5f);
    EXPECT_FLOAT_EQ(blended.y, 0.5f);
    EXPECT_FLOAT_EQ(blended.z, 0.0f);
    EXPECT_FLOAT_EQ(blended.w, 1.0f);
}

// Test predefined colors
TEST_F(ColorTest, PredefinedColors)
{
    Color red = Color::Red();
    Color green = Color::Green();
    Color blue = Color::Blue();

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
TEST_F(ColorTest, BlendVector3)
{
    omath::Vector3 v1(1.0f, 0.0f, 0.0f); // Red
    omath::Vector3 v2(0.0f, 1.0f, 0.0f); // Green
    omath::Vector3 blended = Blend(v1, v2, 0.5f);
    EXPECT_FLOAT_EQ(blended.x, 0.5f);
    EXPECT_FLOAT_EQ(blended.y, 0.5f);
    EXPECT_FLOAT_EQ(blended.z, 0.0f);
}