// Combined color tests
// This file merges multiple color-related unit test files into one grouped TU
// to make the tests look more organized.

#include <gtest/gtest.h>
#include <omath/utility/color.hpp>
#include <format>
#include <algorithm>

using namespace omath;

class UnitTestColorGrouped : public ::testing::Test
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

// From original unit_test_color.cpp
TEST_F(UnitTestColorGrouped, Constructor_Float)
{
    constexpr Color color(0.5f, 0.5f, 0.5f, 1.0f);
    EXPECT_FLOAT_EQ(color.x, 0.5f);
    EXPECT_FLOAT_EQ(color.y, 0.5f);
    EXPECT_FLOAT_EQ(color.z, 0.5f);
    EXPECT_FLOAT_EQ(color.w, 1.0f);
}

TEST_F(UnitTestColorGrouped, Constructor_Vector4)
{
    constexpr omath::Vector4 vec(0.2f, 0.4f, 0.6f, 0.8f);
    constexpr Color color(vec);
    EXPECT_FLOAT_EQ(color.x, 0.2f);
    EXPECT_FLOAT_EQ(color.y, 0.4f);
    EXPECT_FLOAT_EQ(color.z, 0.6f);
    EXPECT_FLOAT_EQ(color.w, 0.8f);
}

TEST_F(UnitTestColorGrouped, FromRGBA)
{
    constexpr Color color = Color::from_rgba(128, 64, 32, 255);
    EXPECT_FLOAT_EQ(color.x, 128.0f / 255.0f);
    EXPECT_FLOAT_EQ(color.y, 64.0f / 255.0f);
    EXPECT_FLOAT_EQ(color.z, 32.0f / 255.0f);
    EXPECT_FLOAT_EQ(color.w, 1.0f);
}

TEST_F(UnitTestColorGrouped, FromHSV)
{
    constexpr Color color = Color::from_hsv(0.0f, 1.0f, 1.0f); // Red in HSV
    EXPECT_FLOAT_EQ(color.x, 1.0f);
    EXPECT_FLOAT_EQ(color.y, 0.0f);
    EXPECT_FLOAT_EQ(color.z, 0.0f);
    EXPECT_FLOAT_EQ(color.w, 1.0f);
}

TEST_F(UnitTestColorGrouped, ToHSV)
{
    const auto [hue, saturation, value] = color1.to_hsv(); // Red color
    EXPECT_FLOAT_EQ(hue, 0.0f);
    EXPECT_FLOAT_EQ(saturation, 1.0f);
    EXPECT_FLOAT_EQ(value, 1.0f);
}

TEST_F(UnitTestColorGrouped, Blend)
{
    const Color blended = color1.blend(color2, 0.5f);
    EXPECT_FLOAT_EQ(blended.x, 0.5f);
    EXPECT_FLOAT_EQ(blended.y, 0.5f);
    EXPECT_FLOAT_EQ(blended.z, 0.0f);
    EXPECT_FLOAT_EQ(blended.w, 1.0f);
}

TEST_F(UnitTestColorGrouped, PredefinedColors)
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

TEST_F(UnitTestColorGrouped, BlendVector3)
{
    constexpr Color v1(1.0f, 0.0f, 0.0f, 1.f); // Red
    constexpr Color v2(0.0f, 1.0f, 0.0f, 1.f); // Green
    constexpr Color blended = v1.blend(v2, 0.5f);
    EXPECT_FLOAT_EQ(blended.x, 0.5f);
    EXPECT_FLOAT_EQ(blended.y, 0.5f);
    EXPECT_FLOAT_EQ(blended.z, 0.0f);
}

// From unit_test_color_extra.cpp
TEST(UnitTestColorGrouped_Extra, SetHueSaturationValue)
{
    Color c = Color::red();
    auto h1 = c.to_hsv();
    EXPECT_FLOAT_EQ(h1.hue, 0.f);

    c.set_hue(0.5f);
    auto h2 = c.to_hsv();
    EXPECT_NEAR(h2.hue, 0.5f, 1e-3f);

    c = Color::from_hsv(0.25f, 0.8f, 0.6f);
    c.set_saturation(0.3f);
    auto h3 = c.to_hsv();
    EXPECT_NEAR(h3.saturation, 0.3f, 1e-3f);

    c.set_value(1.0f);
    auto h4 = c.to_hsv();
    EXPECT_NEAR(h4.value, 1.0f, 1e-3f);
}

TEST(UnitTestColorGrouped_Extra, ToStringVariants)
{
    Color c = Color::from_rgba(10, 20, 30, 255);
    auto s = c.to_string();
    EXPECT_NE(s.find("r:"), std::string::npos);

    auto ws = c.to_wstring();
    EXPECT_FALSE(ws.empty());

    auto u8 = c.to_u8string();
    EXPECT_FALSE(u8.empty());
}

TEST(UnitTestColorGrouped_Extra, BlendEdgeCases)
{
    Color a = Color::red();
    Color b = Color::blue();
    auto r0 = a.blend(b, 0.f);
    EXPECT_FLOAT_EQ(r0.x, a.x);
    auto r1 = a.blend(b, 1.f);
    EXPECT_FLOAT_EQ(r1.x, b.x);
}

// From unit_test_color_more.cpp
TEST(UnitTestColorGrouped_More, DefaultCtorIsZero)
{
    Color c;
    EXPECT_FLOAT_EQ(c.x, 0.0f);
    EXPECT_FLOAT_EQ(c.y, 0.0f);
    EXPECT_FLOAT_EQ(c.z, 0.0f);
    EXPECT_FLOAT_EQ(c.w, 0.0f);
}

TEST(UnitTestColorGrouped_More, FloatCtorAndClampForRGB)
{
    Color c(1.2f, -0.5f, 0.5f, 2.0f);
    EXPECT_FLOAT_EQ(c.x, 1.0f);
    EXPECT_FLOAT_EQ(c.y, 0.0f);
    EXPECT_FLOAT_EQ(c.z, 0.5f);
    EXPECT_FLOAT_EQ(c.w, 2.0f);
}

TEST(UnitTestColorGrouped_More, FromRgbaProducesScaledComponents)
{
    Color c = Color::from_rgba(25u, 128u, 230u, 64u);
    EXPECT_NEAR(c.x, 25.0f/255.0f, 1e-6f);
    EXPECT_NEAR(c.y, 128.0f/255.0f, 1e-6f);
    EXPECT_NEAR(c.z, 230.0f/255.0f, 1e-6f);
    EXPECT_NEAR(c.w, 64.0f/255.0f, 1e-6f);
}

TEST(UnitTestColorGrouped_More, BlendProducesIntermediate)
{
    Color c0(0.0f, 0.0f, 0.0f, 1.0f);
    Color c1(1.0f, 1.0f, 1.0f, 0.0f);
    Color mid = c0.blend(c1, 0.5f);
    EXPECT_FLOAT_EQ(mid.x, 0.5f);
    EXPECT_FLOAT_EQ(mid.y, 0.5f);
    EXPECT_FLOAT_EQ(mid.z, 0.5f);
    EXPECT_FLOAT_EQ(mid.w, 0.5f);
}

TEST(UnitTestColorGrouped_More, HsvRoundTrip)
{
    Color red = Color::red();
    auto hsv = red.to_hsv();
    Color back = Color::from_hsv(hsv);
    EXPECT_NEAR(back.x, 1.0f, 1e-6f);
    EXPECT_NEAR(back.y, 0.0f, 1e-6f);
    EXPECT_NEAR(back.z, 0.0f, 1e-6f);
}

TEST(UnitTestColorGrouped_More, ToStringContainsComponents)
{
    Color c = Color::from_rgba(10, 20, 30, 40);
    std::string s = c.to_string();
    EXPECT_NE(s.find("r:"), std::string::npos);
    EXPECT_NE(s.find("g:"), std::string::npos);
    EXPECT_NE(s.find("b:"), std::string::npos);
    EXPECT_NE(s.find("a:"), std::string::npos);
}

// From unit_test_color_more2.cpp
TEST(UnitTestColorGrouped_More2, FromRgbaAndToString)
{
    auto c = Color::from_rgba(255, 128, 0, 64);
    const auto s = c.to_string();
    EXPECT_NE(s.find("r:255"), std::string::npos);
    EXPECT_NE(s.find("g:128"), std::string::npos);
    EXPECT_NE(s.find("b:0"), std::string::npos);
    EXPECT_NE(s.find("a:64"), std::string::npos);
}

TEST(UnitTestColorGrouped_More2, FromHsvCases)
{
    const float eps = 1e-5f;

    auto check_hue = [&](float h) {
        SCOPED_TRACE(::testing::Message() << "h=" << h);
        Color c = Color::from_hsv(h, 1.f, 1.f);
        EXPECT_TRUE(std::isfinite(c.x));
        EXPECT_TRUE(std::isfinite(c.y));
        EXPECT_TRUE(std::isfinite(c.z));
        EXPECT_GE(c.x, -eps);
        EXPECT_LE(c.x, 1.f + eps);
        EXPECT_GE(c.y, -eps);
        EXPECT_LE(c.y, 1.f + eps);
        EXPECT_GE(c.z, -eps);
        EXPECT_LE(c.z, 1.f + eps);

        float mx = std::max({c.x, c.y, c.z});
        float mn = std::min({c.x, c.y, c.z});
        EXPECT_GE(mx, 0.999f);
        EXPECT_LE(mn, 1e-3f + 1e-4f);
    };

    check_hue(0.f / 6.f);
    check_hue(1.f / 6.f);
    check_hue(2.f / 6.f);
    check_hue(3.f / 6.f);
    check_hue(4.f / 6.f);
    check_hue(5.f / 6.f);
}

TEST(UnitTestColorGrouped_More2, ToHsvAndSetters)
{
    Color c{0.2f, 0.4f, 0.6f, 1.f};
    auto hsv = c.to_hsv();
    EXPECT_NEAR(hsv.value, 0.6f, 1e-6f);

    c.set_hue(0.0f);
    EXPECT_TRUE(std::isfinite(c.x));

    c.set_saturation(0.0f);
    EXPECT_TRUE(std::isfinite(c.y));

    c.set_value(0.5f);
    EXPECT_TRUE(std::isfinite(c.z));
}

TEST(UnitTestColorGrouped_More2, BlendAndStaticColors)
{
    Color a = Color::red();
    Color b = Color::blue();
    auto mid = a.blend(b, 0.5f);
    EXPECT_GT(mid.x, 0.f);
    EXPECT_GT(mid.z, 0.f);

    auto all_a = a.blend(b, -1.f);
    EXPECT_NEAR(all_a.x, a.x, 1e-6f);

    auto all_b = a.blend(b, 2.f);
    EXPECT_NEAR(all_b.z, b.z, 1e-6f);
}

TEST(UnitTestColorGrouped_More2, FormatterUsesToString)
{
    Color c = Color::from_rgba(10, 20, 30, 40);
    const auto formatted = std::format("{}", c);
    EXPECT_NE(formatted.find("r:10"), std::string::npos);
}
