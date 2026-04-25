//
// Created by vlad on 9/24/2024.
//
//
// Vector4Test.cpp
//

#include <omath/linear_algebra/vector4.hpp>
#include <gtest/gtest.h>
#include <limits> // For std::numeric_limits

using namespace omath;

TEST(Vector4More, ConstructorsAndClamp)
{
    constexpr Vector4<float> a;
    EXPECT_EQ(a.x, 0.f);
    EXPECT_EQ(a.y, 0.f);
    EXPECT_EQ(a.z, 0.f);
    EXPECT_EQ(a.w, 0.f);

    Vector4<float> b{1.f, -2.f, 3.5f, 4.f};
    b.clamp(0.f, 3.f);
    EXPECT_GE(b.x, 0.f);
    EXPECT_GE(b.y, 0.f);
    EXPECT_LE(b.z, 3.f);
}

TEST(Vector4More, ComparisonsAndHashFormatter)
{
    constexpr Vector4<int> a{1,2,3,4};
    constexpr Vector4<int> b{1,2,3,5};
    EXPECT_NE(a, b);

    // exercise to_string via formatting if available by converting via std::format
    // call length and comparison to exercise more branches
    EXPECT_LT(a.length(), b.length());
}

class UnitTestVector4 : public ::testing::Test
{
protected:
    Vector4<float> v1;
    Vector4<float> v2;

    void SetUp() override
    {
        v1 = Vector4(1.0f, 2.0f, 3.0f, 4.0f);
        v2 = Vector4(4.0f, 5.0f, 6.0f, 7.0f);
    }
};

// Test constructor and default values
TEST_F(UnitTestVector4, Constructor_Default)
{
    constexpr Vector4<float> v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
    EXPECT_FLOAT_EQ(v.w, 0.0f);
}

TEST_F(UnitTestVector4, Constructor_Values)
{
    constexpr Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v.x, 1.0f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
    EXPECT_FLOAT_EQ(v.z, 3.0f);
    EXPECT_FLOAT_EQ(v.w, 4.0f);
}

// Test equality operators
TEST_F(UnitTestVector4, EqualityOperator)
{
    constexpr Vector4 v3(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_TRUE(v1 == v3);
    EXPECT_FALSE(v1 == v2);
}

TEST_F(UnitTestVector4, InequalityOperator)
{
    constexpr Vector4 v3(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_FALSE(v1 != v3);
    EXPECT_TRUE(v1 != v2);
}

// Test arithmetic operators
TEST_F(UnitTestVector4, AdditionOperator)
{
    constexpr Vector4 v3 = Vector4(1.0f, 2.0f, 3.0f, 4.0f) + Vector4(4.0f, 5.0f, 6.0f, 7.0f);
    EXPECT_FLOAT_EQ(v3.x, 5.0f);
    EXPECT_FLOAT_EQ(v3.y, 7.0f);
    EXPECT_FLOAT_EQ(v3.z, 9.0f);
    EXPECT_FLOAT_EQ(v3.w, 11.0f);
}

TEST_F(UnitTestVector4, SubtractionOperator)
{
    constexpr Vector4 v3 = Vector4(4.0f, 5.0f, 6.0f, 7.0f) - Vector4(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v3.x, 3.0f);
    EXPECT_FLOAT_EQ(v3.y, 3.0f);
    EXPECT_FLOAT_EQ(v3.z, 3.0f);
    EXPECT_FLOAT_EQ(v3.w, 3.0f);
}

TEST_F(UnitTestVector4, MultiplicationOperator_Scalar)
{
    constexpr Vector4 v3 = Vector4(1.0f, 2.0f, 3.0f, 4.0f) * 2.0f;
    EXPECT_FLOAT_EQ(v3.x, 2.0f);
    EXPECT_FLOAT_EQ(v3.y, 4.0f);
    EXPECT_FLOAT_EQ(v3.z, 6.0f);
    EXPECT_FLOAT_EQ(v3.w, 8.0f);
}

TEST_F(UnitTestVector4, MultiplicationOperator_Vector)
{
    constexpr Vector4 v3 = Vector4(1.0f, 2.0f, 3.0f, 4.0f) * Vector4(4.0f, 5.0f, 6.0f, 7.0f);
    EXPECT_FLOAT_EQ(v3.x, 4.0f);
    EXPECT_FLOAT_EQ(v3.y, 10.0f);
    EXPECT_FLOAT_EQ(v3.z, 18.0f);
    EXPECT_FLOAT_EQ(v3.w, 28.0f);
}

TEST_F(UnitTestVector4, DivisionOperator_Scalar)
{
    constexpr Vector4 v3 = Vector4(4.0f, 5.0f, 6.0f, 7.0f) / 2.0f;
    EXPECT_FLOAT_EQ(v3.x, 2.0f);
    EXPECT_FLOAT_EQ(v3.y, 2.5f);
    EXPECT_FLOAT_EQ(v3.z, 3.0f);
    EXPECT_FLOAT_EQ(v3.w, 3.5f);
}

TEST_F(UnitTestVector4, DivisionOperator_Vector)
{
    constexpr Vector4 v3 = Vector4(4.0f, 5.0f, 6.0f, 7.0f) / Vector4(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v3.x, 4.0f);
    EXPECT_FLOAT_EQ(v3.y, 2.5f);
    EXPECT_FLOAT_EQ(v3.z, 2.0f);
    EXPECT_FLOAT_EQ(v3.w, 1.75f);
}

// Test compound assignment operators
TEST_F(UnitTestVector4, AdditionAssignmentOperator)
{
    v1 += v2;
    EXPECT_FLOAT_EQ(v1.x, 5.0f);
    EXPECT_FLOAT_EQ(v1.y, 7.0f);
    EXPECT_FLOAT_EQ(v1.z, 9.0f);
    EXPECT_FLOAT_EQ(v1.w, 11.0f);
}

TEST_F(UnitTestVector4, SubtractionAssignmentOperator)
{
    v1 -= v2;
    EXPECT_FLOAT_EQ(v1.x, -3.0f);
    EXPECT_FLOAT_EQ(v1.y, -3.0f);
    EXPECT_FLOAT_EQ(v1.z, -3.0f);
    EXPECT_FLOAT_EQ(v1.w, -3.0f);
}

TEST_F(UnitTestVector4, MultiplicationAssignmentOperator_Scalar)
{
    v1 *= 2.0f;
    EXPECT_FLOAT_EQ(v1.x, 2.0f);
    EXPECT_FLOAT_EQ(v1.y, 4.0f);
    EXPECT_FLOAT_EQ(v1.z, 6.0f);
    EXPECT_FLOAT_EQ(v1.w, 8.0f);
}

TEST_F(UnitTestVector4, MultiplicationAssignmentOperator_Vector)
{
    v1 *= v2;
    EXPECT_FLOAT_EQ(v1.x, 4.0f);
    EXPECT_FLOAT_EQ(v1.y, 10.0f);
    EXPECT_FLOAT_EQ(v1.z, 18.0f);
    EXPECT_FLOAT_EQ(v1.w, 28.0f);
}

TEST_F(UnitTestVector4, DivisionAssignmentOperator_Scalar)
{
    v1 /= 2.0f;
    EXPECT_FLOAT_EQ(v1.x, 0.5f);
    EXPECT_FLOAT_EQ(v1.y, 1.0f);
    EXPECT_FLOAT_EQ(v1.z, 1.5f);
    EXPECT_FLOAT_EQ(v1.w, 2.0f);
}

TEST_F(UnitTestVector4, DivisionAssignmentOperator_Vector)
{
    v1 /= v2;
    EXPECT_FLOAT_EQ(v1.x, 0.25f);
    EXPECT_FLOAT_EQ(v1.y, 0.4f);
    EXPECT_FLOAT_EQ(v1.z, 0.5f);
    EXPECT_FLOAT_EQ(v1.w, 4.0f / 7.0f);
}

TEST_F(UnitTestVector4, NegationOperator)
{
    constexpr Vector4 v3 = -Vector4(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v3.x, -1.0f);
    EXPECT_FLOAT_EQ(v3.y, -2.0f);
    EXPECT_FLOAT_EQ(v3.z, -3.0f);
    EXPECT_FLOAT_EQ(v3.w, -4.0f);
}

// Test other member functions
TEST_F(UnitTestVector4, LengthSqr)
{
    constexpr float lengthSqr = Vector4(1.0f, 2.0f, 3.0f, 4.0f).length_sqr();
    EXPECT_FLOAT_EQ(lengthSqr, 30.0f);
}

TEST_F(UnitTestVector4, DotProduct)
{
    constexpr float dot = Vector4(1.0f, 2.0f, 3.0f, 4.0f).dot(Vector4(4.0f, 5.0f, 6.0f, 7.0f));
    EXPECT_FLOAT_EQ(dot, 60.0f);
}

TEST_F(UnitTestVector4, Abs)
{
    Vector4 v3 = Vector4(-1.0f, -2.0f, -3.0f, -4.0f);
    v3.abs();
    EXPECT_FLOAT_EQ(v3.x, 1.0f);
    EXPECT_FLOAT_EQ(v3.y, 2.0f);
    EXPECT_FLOAT_EQ(v3.z, 3.0f);
    EXPECT_FLOAT_EQ(v3.w, 4.0f);
}

TEST_F(UnitTestVector4, Sum)
{
    constexpr float sum = Vector4(1.0f, 2.0f, 3.0f, 4.0f).sum();
    EXPECT_FLOAT_EQ(sum, 10.0f);
}

TEST_F(UnitTestVector4, Clamp)
{
    Vector4 v3 = Vector4(1.0f, 2.0f, 3.0f, 4.0f);
    v3.clamp(1.5f, 2.5f);
    EXPECT_FLOAT_EQ(v3.x, 1.5f);
    EXPECT_FLOAT_EQ(v3.y, 2.0f);
    EXPECT_FLOAT_EQ(v3.z, 2.5f);
    EXPECT_FLOAT_EQ(v3.w, 4.0f); // w is not clamped in this method
}
TEST_F(UnitTestVector4, LessOperator)
{
    EXPECT_TRUE(v1 < v2);
}

TEST_F(UnitTestVector4, GreaterOperator)
{
    EXPECT_TRUE(v2 > v1);
}
TEST_F(UnitTestVector4, LessEqualOperator)
{
    EXPECT_TRUE(omath::Vector4<float>{} <= omath::Vector4<float>{});
    EXPECT_TRUE(omath::Vector4<float>{} <= omath::Vector4(1.f, 1.f, 1.f, 1.f));
}

TEST_F(UnitTestVector4, GreaterEqualOperator)
{
    EXPECT_TRUE(omath::Vector4<float>{} >= omath::Vector4<float>{});
    EXPECT_TRUE(omath::Vector4(1.f, 1.f, 1.f, 1.f) >= omath::Vector4<float>{});
}

// ── Cast operator tests ──────────────────────────────────────────────────────

using namespace omath;

TEST(Vector4Cast, FloatToDouble)
{
    constexpr Vector4<float> v{1.5f, -2.5f, 3.0f, 4.25f};
    constexpr auto result = static_cast<Vector4<double>>(v);
    EXPECT_DOUBLE_EQ(result.x, 1.5);
    EXPECT_DOUBLE_EQ(result.y, -2.5);
    EXPECT_DOUBLE_EQ(result.z, 3.0);
    EXPECT_DOUBLE_EQ(result.w, 4.25);
}

TEST(Vector4Cast, DoubleToFloat)
{
    constexpr Vector4<double> v{1.25, -3.75, 0.5, -0.125};
    constexpr auto result = static_cast<Vector4<float>>(v);
    EXPECT_FLOAT_EQ(result.x, 1.25f);
    EXPECT_FLOAT_EQ(result.y, -3.75f);
    EXPECT_FLOAT_EQ(result.z, 0.5f);
    EXPECT_FLOAT_EQ(result.w, -0.125f);
}

TEST(Vector4Cast, FloatToInt_Truncates)
{
    constexpr Vector4<float> v{3.9f, -2.1f, 7.7f, -0.9f};
    constexpr auto result = static_cast<Vector4<int>>(v);
    EXPECT_EQ(result.x, 3);
    EXPECT_EQ(result.y, -2);
    EXPECT_EQ(result.z, 7);
    EXPECT_EQ(result.w, 0);
}

TEST(Vector4Cast, IntToFloat_Exact)
{
    constexpr Vector4<int> v{7, -4, 0, 1};
    constexpr auto result = static_cast<Vector4<float>>(v);
    EXPECT_FLOAT_EQ(result.x, 7.f);
    EXPECT_FLOAT_EQ(result.y, -4.f);
    EXPECT_FLOAT_EQ(result.z, 0.f);
    EXPECT_FLOAT_EQ(result.w, 1.f);
}

TEST(Vector4Cast, ZeroPreserved)
{
    constexpr Vector4<float> v{0.f, 0.f, 0.f, 0.f};
    constexpr auto result = static_cast<Vector4<double>>(v);
    EXPECT_DOUBLE_EQ(result.x, 0.0);
    EXPECT_DOUBLE_EQ(result.y, 0.0);
    EXPECT_DOUBLE_EQ(result.z, 0.0);
    EXPECT_DOUBLE_EQ(result.w, 0.0);
}

TEST(Vector4Cast, NegativeValues)
{
    constexpr Vector4<double> v{-100.0, -200.0, -300.0, -400.0};
    constexpr auto result = static_cast<Vector4<float>>(v);
    EXPECT_FLOAT_EQ(result.x, -100.f);
    EXPECT_FLOAT_EQ(result.y, -200.f);
    EXPECT_FLOAT_EQ(result.z, -300.f);
    EXPECT_FLOAT_EQ(result.w, -400.f);
}

TEST(Vector4Cast, SameTypeRoundtrip)
{
    constexpr Vector4<float> v{1.f, 2.f, 3.f, 4.f};
    constexpr auto result = static_cast<Vector4<float>>(v);
    EXPECT_FLOAT_EQ(result.x, v.x);
    EXPECT_FLOAT_EQ(result.y, v.y);
    EXPECT_FLOAT_EQ(result.z, v.z);
    EXPECT_FLOAT_EQ(result.w, v.w);
}