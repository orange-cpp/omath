//
// Created by Vlad on 02.09.2024.
//

#include <gtest/gtest.h>
#include <omath/Vector2.h>
#include <cmath> // For std::isinf and std::isnan
#include <cfloat> // For FLT_MAX and FLT_MIN

using namespace omath;

class UnitTestVector2 : public ::testing::Test
{
protected:
    Vector2 v1;
    Vector2 v2;

    constexpr void SetUp() override
    {
        v1 = Vector2(1.0f, 2.0f);
        v2 = Vector2(4.0f, 5.0f);
    }
};

// Test constructor and default values
TEST_F(UnitTestVector2, Constructor_Default)
{
    constexpr Vector2 v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
}

TEST_F(UnitTestVector2, Constructor_Values)
{
    constexpr Vector2 v(1.0f, 2.0f);
    EXPECT_FLOAT_EQ(v.x, 1.0f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
}

// Test equality operators
TEST_F(UnitTestVector2, EqualityOperator)
{
    constexpr Vector2 v3(1.0f, 2.0f);
    EXPECT_TRUE(v1 == v3);
    EXPECT_FALSE(v1 == v2);
}

TEST_F(UnitTestVector2, InequalityOperator)
{
    constexpr Vector2 v3(1.0f, 2.0f);
    EXPECT_FALSE(v1 != v3);
    EXPECT_TRUE(v1 != v2);
}

// Test arithmetic operators
TEST_F(UnitTestVector2, AdditionOperator)
{
    constexpr Vector2 v3 = Vector2(1.0f, 2.0f) + Vector2(4.0f, 5.0f);
    EXPECT_FLOAT_EQ(v3.x, 5.0f);
    EXPECT_FLOAT_EQ(v3.y, 7.0f);
}

TEST_F(UnitTestVector2, SubtractionOperator)
{
    constexpr Vector2 v3 = Vector2(4.0f, 5.0f) - Vector2(1.0f, 2.0f);
    EXPECT_FLOAT_EQ(v3.x, 3.0f);
    EXPECT_FLOAT_EQ(v3.y, 3.0f);
}

TEST_F(UnitTestVector2, MultiplicationOperator)
{
    constexpr Vector2 v3 = Vector2(1.0f, 2.0f) * 2.0f;
    EXPECT_FLOAT_EQ(v3.x, 2.0f);
    EXPECT_FLOAT_EQ(v3.y, 4.0f);
}

TEST_F(UnitTestVector2, DivisionOperator)
{
    constexpr Vector2 v3 = Vector2(4.0f, 5.0f) / 2.0f;
    EXPECT_FLOAT_EQ(v3.x, 2.0f);
    EXPECT_FLOAT_EQ(v3.y, 2.5f);
}

TEST_F(UnitTestVector2, NegationOperator)
{
    constexpr Vector2 v3 = -Vector2(1.0f, 2.0f);
    EXPECT_FLOAT_EQ(v3.x, -1.0f);
    EXPECT_FLOAT_EQ(v3.y, -2.0f);
}

// Test compound assignment operators
TEST_F(UnitTestVector2, AdditionAssignmentOperator)
{
    v1 += v2;
    EXPECT_FLOAT_EQ(v1.x, 5.0f);
    EXPECT_FLOAT_EQ(v1.y, 7.0f);
}

TEST_F(UnitTestVector2, SubtractionAssignmentOperator)
{
    v1 -= v2;
    EXPECT_FLOAT_EQ(v1.x, -3.0f);
    EXPECT_FLOAT_EQ(v1.y, -3.0f);
}

TEST_F(UnitTestVector2, MultiplicationAssignmentOperator)
{
    v1 *= 2.0f;
    EXPECT_FLOAT_EQ(v1.x, 2.0f);
    EXPECT_FLOAT_EQ(v1.y, 4.0f);
}

TEST_F(UnitTestVector2, DivisionAssignmentOperator)
{
    v1 /= 2.0f;
    EXPECT_FLOAT_EQ(v1.x, 0.5f);
    EXPECT_FLOAT_EQ(v1.y, 1.0f);
}

// New tests for compound assignment with vectors
TEST_F(UnitTestVector2, MultiplicationAssignmentOperator_Vector)
{
    v1 *= v2;
    EXPECT_FLOAT_EQ(v1.x, 1.0f * 4.0f);
    EXPECT_FLOAT_EQ(v1.y, 2.0f * 5.0f);
}

TEST_F(UnitTestVector2, DivisionAssignmentOperator_Vector)
{
    v1 /= v2;
    EXPECT_FLOAT_EQ(v1.x, 1.0f / 4.0f);
    EXPECT_FLOAT_EQ(v1.y, 2.0f / 5.0f);
}

// New tests for compound assignment with floats
TEST_F(UnitTestVector2, AdditionAssignmentOperator_Float)
{
    v1 += 3.0f;
    EXPECT_FLOAT_EQ(v1.x, 4.0f);
    EXPECT_FLOAT_EQ(v1.y, 5.0f);
}

TEST_F(UnitTestVector2, SubtractionAssignmentOperator_Float)
{
    v1 -= 1.0f;
    EXPECT_FLOAT_EQ(v1.x, 0.0f);
    EXPECT_FLOAT_EQ(v1.y, 1.0f);
}

// Test other member functions
TEST_F(UnitTestVector2, DistTo)
{
    const float dist = v1.DistTo(v2);
    EXPECT_FLOAT_EQ(dist, std::sqrt(18.0f));
}

TEST_F(UnitTestVector2, DistTo_SamePoint)
{
    const float dist = v1.DistTo(v1);
    EXPECT_FLOAT_EQ(dist, 0.0f);
}

TEST_F(UnitTestVector2, DistToSqr)
{
    constexpr float distSqr = Vector2(1.0f, 2.0f).DistToSqr(Vector2(4.0f, 5.0f));
    EXPECT_FLOAT_EQ(distSqr, 18.0f);
}

TEST_F(UnitTestVector2, DistToSqr_SamePoint)
{
    constexpr float distSqr = Vector2(1.0f, 2.0f).DistToSqr(Vector2(1.0f, 2.0f));
    EXPECT_FLOAT_EQ(distSqr, 0.0f);
}

TEST_F(UnitTestVector2, DotProduct)
{
    constexpr float dot = Vector2(1.0f, 2.0f).Dot(Vector2(4.0f, 5.0f));
    EXPECT_FLOAT_EQ(dot, 14.0f);
}

TEST_F(UnitTestVector2, DotProduct_PerpendicularVectors)
{
    constexpr float dot = Vector2(1.0f, 0.0f).Dot(Vector2(0.0f, 1.0f));
    EXPECT_FLOAT_EQ(dot, 0.0f);
}

TEST_F(UnitTestVector2, DotProduct_ParallelVectors)
{
    constexpr float dot = Vector2(1.0f, 1.0f).Dot(Vector2(2.0f, 2.0f));
    EXPECT_FLOAT_EQ(dot, 4.0f);
}

TEST_F(UnitTestVector2, Length)
{
    const float length = v1.Length();
    EXPECT_FLOAT_EQ(length, std::sqrt(5.0f));
}

TEST_F(UnitTestVector2, Length_ZeroVector)
{
    constexpr Vector2 v_zero(0.0f, 0.0f);
    const float length = v_zero.Length();
    EXPECT_FLOAT_EQ(length, 0.0f);
}

TEST_F(UnitTestVector2, Length_LargeValues)
{
    constexpr Vector2 v_large(FLT_MAX, FLT_MAX);
    const float length = v_large.Length();
    EXPECT_TRUE(std::isinf(length));
}

TEST_F(UnitTestVector2, LengthSqr)
{
    constexpr float lengthSqr = Vector2(1.0f, 2.0f).LengthSqr();
    EXPECT_FLOAT_EQ(lengthSqr, 5.0f);
}

TEST_F(UnitTestVector2, Abs)
{
    Vector2 v3(-1.0f, -2.0f);
    v3.Abs();
    EXPECT_FLOAT_EQ(v3.x, 1.0f);
    EXPECT_FLOAT_EQ(v3.y, 2.0f);
}

TEST_F(UnitTestVector2, Abs_PositiveValues)
{
    Vector2 v3(1.0f, 2.0f);
    v3.Abs();
    EXPECT_FLOAT_EQ(v3.x, 1.0f);
    EXPECT_FLOAT_EQ(v3.y, 2.0f);
}

TEST_F(UnitTestVector2, Abs_ZeroValues)
{
    Vector2 v3(0.0f, 0.0f);
    v3.Abs();
    EXPECT_FLOAT_EQ(v3.x, 0.0f);
    EXPECT_FLOAT_EQ(v3.y, 0.0f);
}

TEST_F(UnitTestVector2, Sum)
{
    constexpr float sum = Vector2(1.0f, 2.0f).Sum();
    EXPECT_FLOAT_EQ(sum, 3.0f);
}

TEST_F(UnitTestVector2, Sum_NegativeValues)
{
    constexpr float sum = Vector2(-1.0f, -2.0f).Sum();
    EXPECT_FLOAT_EQ(sum, -3.0f);
}

TEST_F(UnitTestVector2, Normalized)
{
    const Vector2 v3 = v1.Normalized();
    EXPECT_NEAR(v3.x, 0.44721f, 0.0001f);
    EXPECT_NEAR(v3.y, 0.89443f, 0.0001f);
}

TEST_F(UnitTestVector2, Normalized_ZeroVector)
{
    constexpr Vector2 v_zero(0.0f, 0.0f);
    const Vector2 v_norm = v_zero.Normalized();
    EXPECT_FLOAT_EQ(v_norm.x, 0.0f);
    EXPECT_FLOAT_EQ(v_norm.y, 0.0f);
}

// Test AsTuple method
TEST_F(UnitTestVector2, AsTuple)
{
    const auto tuple = v1.AsTuple();
    EXPECT_FLOAT_EQ(std::get<0>(tuple), v1.x);
    EXPECT_FLOAT_EQ(std::get<1>(tuple), v1.y);
}

// Test division by zero
TEST_F(UnitTestVector2, DivisionOperator_DivideByZero)
{
    constexpr Vector2 v(1.0f, 2.0f);
    constexpr float zero = 0.0f;
    const Vector2 result = v / zero;
    EXPECT_TRUE(std::isinf(result.x) || std::isnan(result.x));
    EXPECT_TRUE(std::isinf(result.y) || std::isnan(result.y));
}

TEST_F(UnitTestVector2, DivisionAssignmentOperator_DivideByZero)
{
    Vector2 v(1.0f, 2.0f);
    constexpr float zero = 0.0f;
    v /= zero;
    EXPECT_TRUE(std::isinf(v.x) || std::isnan(v.x));
    EXPECT_TRUE(std::isinf(v.y) || std::isnan(v.y));
}

TEST_F(UnitTestVector2, DivisionAssignmentOperator_VectorWithZero)
{
    Vector2 v(1.0f, 2.0f);
    constexpr Vector2 v_zero(0.0f, 1.0f);
    v /= v_zero;
    EXPECT_TRUE(std::isinf(v.x) || std::isnan(v.x));
    EXPECT_FLOAT_EQ(v.y, 2.0f / 1.0f);
}

// Test operations with infinity and NaN
TEST_F(UnitTestVector2, Operator_WithInfinity)
{
    constexpr Vector2 v_inf(INFINITY, INFINITY);
    const Vector2 result = v1 + v_inf;
    EXPECT_TRUE(std::isinf(result.x));
    EXPECT_TRUE(std::isinf(result.y));
}

TEST_F(UnitTestVector2, Operator_WithNaN)
{
    constexpr Vector2 v_nan(NAN, NAN);
    const Vector2 result = v1 + v_nan;
    EXPECT_TRUE(std::isnan(result.x));
    EXPECT_TRUE(std::isnan(result.y));
}

// Test negative values in arithmetic operations
TEST_F(UnitTestVector2, AdditionOperator_NegativeValues)
{
    constexpr Vector2 v_neg(-1.0f, -2.0f);
    const Vector2 result = v1 + v_neg;
    EXPECT_FLOAT_EQ(result.x, 0.0f);
    EXPECT_FLOAT_EQ(result.y, 0.0f);
}

TEST_F(UnitTestVector2, SubtractionOperator_NegativeValues)
{
    constexpr Vector2 v_neg(-1.0f, -2.0f);
    const Vector2 result = v1 - v_neg;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
}

// Test negation of zero vector
TEST_F(UnitTestVector2, NegationOperator_ZeroVector)
{
    constexpr Vector2 v_zero(0.0f, 0.0f);
    constexpr Vector2 result = -v_zero;
    EXPECT_FLOAT_EQ(result.x, -0.0f);
    EXPECT_FLOAT_EQ(result.y, -0.0f);
}

// Static assertions (compile-time checks)
static_assert(Vector2(1.0f, 2.0f).LengthSqr() == 5.0f, "LengthSqr should be 5");
static_assert(Vector2(1.0f, 2.0f).Dot(Vector2(4.0f, 5.0f)) == 14.0f, "Dot product should be 14");
static_assert(Vector2(4.0f, 5.0f).DistToSqr(Vector2(1.0f, 2.0f)) == 18.0f, "DistToSqr should be 18");
static_assert(Vector2(-1.0f, -2.0f).Abs() == Vector2(1.0f, 2.0f), "Abs should convert negative values to positive");
