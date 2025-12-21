//
// Created by Vlad on 01.09.2024.
//

#include <omath/linear_algebra/vector3.hpp>
#include <cfloat> // For FLT_MAX, FLT_MIN
#include <cmath>
#include <gtest/gtest.h>
#include <limits> // For std::numeric_limits

using namespace omath;

class UnitTestVector3 : public ::testing::Test
{
protected:
    Vector3<float> v1;
    Vector3<float> v2;

    void SetUp() override
    {
        v1 = Vector3(1.0f, 2.0f, 3.0f);
        v2 = Vector3(4.0f, 5.0f, 6.0f);
    }
};

// Test constructor and default values
TEST_F(UnitTestVector3, Constructor_Default)
{
    constexpr Vector3<float> v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST_F(UnitTestVector3, Constructor_Values)
{
    constexpr Vector3<float> v(1.0f, 2.0f, 3.0f);
    EXPECT_FLOAT_EQ(v.x, 1.0f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
    EXPECT_FLOAT_EQ(v.z, 3.0f);
}

// Test equality operators
TEST_F(UnitTestVector3, EqualityOperator)
{
    constexpr Vector3 v3(1.0f, 2.0f, 3.0f);
    EXPECT_TRUE(v1 == v3);
    EXPECT_FALSE(v1 == v2);
}

TEST_F(UnitTestVector3, InequalityOperator)
{
    constexpr Vector3 v3(1.0f, 2.0f, 3.0f);
    EXPECT_FALSE(v1 != v3);
    EXPECT_TRUE(v1 != v2);
}

// Test arithmetic operators
TEST_F(UnitTestVector3, AdditionOperator)
{
    constexpr Vector3 v3 = Vector3(1.0f, 2.0f, 3.0f) + Vector3(4.0f, 5.0f, 6.0f);
    EXPECT_FLOAT_EQ(v3.x, 5.0f);
    EXPECT_FLOAT_EQ(v3.y, 7.0f);
    EXPECT_FLOAT_EQ(v3.z, 9.0f);
}

TEST_F(UnitTestVector3, SubtractionOperator)
{
    constexpr Vector3 v3 = Vector3(4.0f, 5.0f, 6.0f) - Vector3(1.0f, 2.0f, 3.0f);
    EXPECT_FLOAT_EQ(v3.x, 3.0f);
    EXPECT_FLOAT_EQ(v3.y, 3.0f);
    EXPECT_FLOAT_EQ(v3.z, 3.0f);
}

TEST_F(UnitTestVector3, MultiplicationOperator_Scalar)
{
    constexpr Vector3 v3 = Vector3(1.0f, 2.0f, 3.0f) * 2.0f;
    EXPECT_FLOAT_EQ(v3.x, 2.0f);
    EXPECT_FLOAT_EQ(v3.y, 4.0f);
    EXPECT_FLOAT_EQ(v3.z, 6.0f);
}

TEST_F(UnitTestVector3, MultiplicationOperator_Vector)
{
    constexpr auto v3 = Vector3(1.0f, 2.0f, 3.0f) * Vector3(4.0f, 5.0f, 6.0f);
    EXPECT_FLOAT_EQ(v3.x, 4.0f);
    EXPECT_FLOAT_EQ(v3.y, 10.0f);
    EXPECT_FLOAT_EQ(v3.z, 18.0f);
}

TEST_F(UnitTestVector3, DivisionOperator_Scalar)
{
    constexpr auto v3 = Vector3(4.0f, 5.0f, 6.0f) / 2.0f;
    EXPECT_FLOAT_EQ(v3.x, 2.0f);
    EXPECT_FLOAT_EQ(v3.y, 2.5f);
    EXPECT_FLOAT_EQ(v3.z, 3.0f);
}

TEST_F(UnitTestVector3, DivisionOperator_Vector)
{
    constexpr auto v3 = Vector3(4.0f, 5.0f, 6.0f) / Vector3(1.0f, 2.0f, 3.0f);
    EXPECT_FLOAT_EQ(v3.x, 4.0f);
    EXPECT_FLOAT_EQ(v3.y, 2.5f);
    EXPECT_FLOAT_EQ(v3.z, 2.0f);
}

// Test compound assignment operators
TEST_F(UnitTestVector3, AdditionAssignmentOperator)
{
    v1 += v2;
    EXPECT_FLOAT_EQ(v1.x, 5.0f);
    EXPECT_FLOAT_EQ(v1.y, 7.0f);
    EXPECT_FLOAT_EQ(v1.z, 9.0f);
}

TEST_F(UnitTestVector3, SubtractionAssignmentOperator)
{
    v1 -= v2;
    EXPECT_FLOAT_EQ(v1.x, -3.0f);
    EXPECT_FLOAT_EQ(v1.y, -3.0f);
    EXPECT_FLOAT_EQ(v1.z, -3.0f);
}

TEST_F(UnitTestVector3, MultiplicationAssignmentOperator_Scalar)
{
    v1 *= 2.0f;
    EXPECT_FLOAT_EQ(v1.x, 2.0f);
    EXPECT_FLOAT_EQ(v1.y, 4.0f);
    EXPECT_FLOAT_EQ(v1.z, 6.0f);
}

TEST_F(UnitTestVector3, MultiplicationAssignmentOperator_Vector)
{
    v1 *= v2;
    EXPECT_FLOAT_EQ(v1.x, 4.0f);
    EXPECT_FLOAT_EQ(v1.y, 10.0f);
    EXPECT_FLOAT_EQ(v1.z, 18.0f);
}

TEST_F(UnitTestVector3, DivisionAssignmentOperator_Scalar)
{
    v1 /= 2.0f;
    EXPECT_FLOAT_EQ(v1.x, 0.5f);
    EXPECT_FLOAT_EQ(v1.y, 1.0f);
    EXPECT_FLOAT_EQ(v1.z, 1.5f);
}

TEST_F(UnitTestVector3, DivisionAssignmentOperator_Vector)
{
    v1 /= v2;
    EXPECT_FLOAT_EQ(v1.x, 0.25f);
    EXPECT_FLOAT_EQ(v1.y, 0.4f);
    EXPECT_FLOAT_EQ(v1.z, 0.5f);
}

TEST_F(UnitTestVector3, NegationOperator)
{
    constexpr auto v3 = -Vector3(1.0f, 2.0f, 3.0f);
    EXPECT_FLOAT_EQ(v3.x, -1.0f);
    EXPECT_FLOAT_EQ(v3.y, -2.0f);
    EXPECT_FLOAT_EQ(v3.z, -3.0f);
}

// Test other member functions
TEST_F(UnitTestVector3, DistToSqr)
{
    constexpr auto distSqr = Vector3(1.0f, 2.0f, 3.0f).distance_to_sqr(Vector3(4.0f, 5.0f, 6.0f));
    EXPECT_FLOAT_EQ(distSqr, 27.0f);
}

TEST_F(UnitTestVector3, DotProduct)
{
    constexpr auto dot = Vector3(1.0f, 2.0f, 3.0f).dot(Vector3(4.0f, 5.0f, 6.0f));
    EXPECT_FLOAT_EQ(dot, 32.0f);
}

TEST_F(UnitTestVector3, LengthSqr)
{
    constexpr auto lengthSqr = Vector3(1.0f, 2.0f, 3.0f).length_sqr();
    EXPECT_FLOAT_EQ(lengthSqr, 14.0f);
}

TEST_F(UnitTestVector3, Abs)
{
    auto v3 = Vector3(-1.0f, -2.0f, -3.0f);
    v3.abs();
    EXPECT_FLOAT_EQ(v3.x, 1.0f);
    EXPECT_FLOAT_EQ(v3.y, 2.0f);
    EXPECT_FLOAT_EQ(v3.z, 3.0f);
}

TEST_F(UnitTestVector3, Sum)
{
    constexpr auto sum = Vector3(1.0f, 2.0f, 3.0f).sum();
    EXPECT_FLOAT_EQ(sum, 6.0f);
}

TEST_F(UnitTestVector3, Sum2D)
{
    constexpr auto sum2D = Vector3(1.0f, 2.0f, 3.0f).sum_2d();
    EXPECT_FLOAT_EQ(sum2D, 3.0f);
}

TEST_F(UnitTestVector3, CrossProduct)
{
    constexpr Vector3 v3 = Vector3(1.0f, 2.0f, 3.0f).cross(Vector3(4.0f, 5.0f, 6.0f));
    EXPECT_FLOAT_EQ(v3.x, -3.0f);
    EXPECT_FLOAT_EQ(v3.y, 6.0f);
    EXPECT_FLOAT_EQ(v3.z, -3.0f);
}

// New tests to cover corner cases

// Test operations with zero vectors
TEST_F(UnitTestVector3, Addition_WithZeroVector)
{
    constexpr Vector3 v_zero(0.0f, 0.0f, 0.0f);
    const Vector3 result = v1 + v_zero;
    EXPECT_FLOAT_EQ(result.x, v1.x);
    EXPECT_FLOAT_EQ(result.y, v1.y);
    EXPECT_FLOAT_EQ(result.z, v1.z);
}

TEST_F(UnitTestVector3, Subtraction_WithZeroVector)
{
    constexpr Vector3 v_zero(0.0f, 0.0f, 0.0f);
    const Vector3 result = v1 - v_zero;
    EXPECT_FLOAT_EQ(result.x, v1.x);
    EXPECT_FLOAT_EQ(result.y, v1.y);
    EXPECT_FLOAT_EQ(result.z, v1.z);
}

TEST_F(UnitTestVector3, Multiplication_WithZeroVector)
{
    constexpr Vector3 v_zero(0.0f, 0.0f, 0.0f);
    const Vector3 result = v1 * v_zero;
    EXPECT_FLOAT_EQ(result.x, 0.0f);
    EXPECT_FLOAT_EQ(result.y, 0.0f);
    EXPECT_FLOAT_EQ(result.z, 0.0f);
}

TEST_F(UnitTestVector3, Division_ByZeroVector)
{
    constexpr Vector3 v_zero(0.0f, 0.0f, 0.0f);
    const Vector3 result = v1 / v_zero;
    EXPECT_TRUE(std::isinf(result.x) || std::isnan(result.x));
    EXPECT_TRUE(std::isinf(result.y) || std::isnan(result.y));
    EXPECT_TRUE(std::isinf(result.z) || std::isnan(result.z));
}

TEST_F(UnitTestVector3, Division_ByZeroScalar)
{
    constexpr float zero = 0.0f;
    const Vector3 result = v1 / zero;
    EXPECT_TRUE(std::isinf(result.x) || std::isnan(result.x));
    EXPECT_TRUE(std::isinf(result.y) || std::isnan(result.y));
    EXPECT_TRUE(std::isinf(result.z) || std::isnan(result.z));
}

// Test operations with infinity
TEST_F(UnitTestVector3, Addition_WithInfinity)
{
    constexpr Vector3 v_inf(INFINITY, INFINITY, INFINITY);
    const Vector3 result = v1 + v_inf;
    EXPECT_TRUE(std::isinf(result.x));
    EXPECT_TRUE(std::isinf(result.y));
    EXPECT_TRUE(std::isinf(result.z));
}

TEST_F(UnitTestVector3, Subtraction_WithInfinity)
{
    constexpr Vector3 v_inf(INFINITY, INFINITY, INFINITY);
    const Vector3 result = v1 - v_inf;
    EXPECT_TRUE(std::isinf(result.x));
    EXPECT_TRUE(std::isinf(result.y));
    EXPECT_TRUE(std::isinf(result.z));
}

// Test operations with NaN
TEST_F(UnitTestVector3, Multiplication_WithNaN)
{
    constexpr Vector3 v_nan(NAN, NAN, NAN);
    const Vector3 result = v1 * v_nan;
    EXPECT_TRUE(std::isnan(result.x));
    EXPECT_TRUE(std::isnan(result.y));
    EXPECT_TRUE(std::isnan(result.z));
}

TEST_F(UnitTestVector3, Division_WithNaN)
{
    constexpr Vector3 v_nan(NAN, NAN, NAN);
    const Vector3 result = v1 / v_nan;
    EXPECT_TRUE(std::isnan(result.x));
    EXPECT_TRUE(std::isnan(result.y));
    EXPECT_TRUE(std::isnan(result.z));
}

// Test Length, Length2D, and Normalized
TEST_F(UnitTestVector3, Length)
{
    const float length = v1.length();
    EXPECT_FLOAT_EQ(length, std::sqrt(14.0f));
}

TEST_F(UnitTestVector3, Length_ZeroVector)
{
    constexpr Vector3 v_zero(0.0f, 0.0f, 0.0f);
    const float length = v_zero.length();
    EXPECT_FLOAT_EQ(length, 0.0f);
}

TEST_F(UnitTestVector3, Length_LargeValues)
{
    constexpr Vector3 v_large(FLT_MAX, FLT_MAX, FLT_MAX);
    const float length = v_large.length();
    EXPECT_TRUE(std::isinf(length));
}

TEST_F(UnitTestVector3, Length2D)
{
    const float length2D = v1.length_2d();
    EXPECT_FLOAT_EQ(length2D, std::sqrt(5.0f));
}

TEST_F(UnitTestVector3, Normalized)
{
    const Vector3 v_norm = v1.normalized();
    const float length = v_norm.length();
    EXPECT_NEAR(length, 1.0f, 0.0001f);
}

TEST_F(UnitTestVector3, Normalized_ZeroVector)
{
    constexpr Vector3 v_zero(0.0f, 0.0f, 0.0f);
    const Vector3 v_norm = v_zero.normalized();
    EXPECT_FLOAT_EQ(v_norm.x, 0.0f);
    EXPECT_FLOAT_EQ(v_norm.y, 0.0f);
    EXPECT_FLOAT_EQ(v_norm.z, 0.0f);
}

// Test Cross Product edge cases
TEST_F(UnitTestVector3, CrossProduct_ParallelVectors)
{
    constexpr Vector3 v_a(1.0f, 2.0f, 3.0f);
    constexpr Vector3 v_b = v_a * 2.0f; // Parallel to v_a
    constexpr Vector3 cross = v_a.cross(v_b);
    EXPECT_FLOAT_EQ(cross.x, 0.0f);
    EXPECT_FLOAT_EQ(cross.y, 0.0f);
    EXPECT_FLOAT_EQ(cross.z, 0.0f);
}

TEST_F(UnitTestVector3, CrossProduct_OrthogonalVectors)
{
    constexpr Vector3 v_a(1.0f, 0.0f, 0.0f);
    constexpr Vector3 v_b(0.0f, 1.0f, 0.0f);
    constexpr Vector3 cross = v_a.cross(v_b);
    EXPECT_FLOAT_EQ(cross.x, 0.0f);
    EXPECT_FLOAT_EQ(cross.y, 0.0f);
    EXPECT_FLOAT_EQ(cross.z, 1.0f);
}

// Test negative values
TEST_F(UnitTestVector3, Addition_NegativeValues)
{
    constexpr Vector3 v_neg(-1.0f, -2.0f, -3.0f);
    const Vector3 result = v1 + v_neg;
    EXPECT_FLOAT_EQ(result.x, 0.0f);
    EXPECT_FLOAT_EQ(result.y, 0.0f);
    EXPECT_FLOAT_EQ(result.z, 0.0f);
}

TEST_F(UnitTestVector3, Subtraction_NegativeValues)
{
    constexpr Vector3 v_neg(-1.0f, -2.0f, -3.0f);
    const Vector3 result = v1 - v_neg;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 6.0f);
}

// Test AsTuple method
TEST_F(UnitTestVector3, AsTuple)
{
    const auto tuple = v1.as_tuple();
    EXPECT_FLOAT_EQ(std::get<0>(tuple), v1.x);
    EXPECT_FLOAT_EQ(std::get<1>(tuple), v1.y);
    EXPECT_FLOAT_EQ(std::get<2>(tuple), v1.z);
}

// Test AsTuple method
TEST_F(UnitTestVector3, AngleBeatween)
{
    EXPECT_NEAR(Vector3(0.0f, 0.0f, 1.0f).angle_between({1, 0, 0}).value().as_degrees(),
                90.0f, 0.001f);
    EXPECT_NEAR(Vector3(0.0f, 0.0f, 1.0f).angle_between({0.0f, 0.0f, 1.0f}).value().as_degrees(),
                0.0f, 0.001f);
    EXPECT_FALSE(Vector3(0.0f, 0.0f, 0.0f).angle_between({0.0f, 0.0f, 1.0f}).has_value());
}

TEST_F(UnitTestVector3, IsPerpendicular)
{
    EXPECT_EQ(Vector3(0.0f, 0.0f, 1.0f).is_perpendicular({1, 0 ,0}), true);
    EXPECT_EQ(Vector3(0.0f, 0.0f, 1.0f).is_perpendicular({0.0f, 0.0f, 1.0f}), false);
    EXPECT_FALSE(Vector3(0.0f, 0.0f, 0.0f).is_perpendicular({0.0f, 0.0f, 1.0f}));
}

TEST_F(UnitTestVector3, LessOperator)
{
    EXPECT_TRUE(v1 < v2);
}

TEST_F(UnitTestVector3, GreaterOperator)
{
    EXPECT_TRUE(v2 > v1);
}
TEST_F(UnitTestVector3, LessEqualOperator)
{
    EXPECT_TRUE(omath::Vector3<float>{} <= omath::Vector3<float>{});
    EXPECT_TRUE(omath::Vector3<float>{} <= omath::Vector3(1.f, 1.f, 1.f));
}

TEST_F(UnitTestVector3, GreaterEqualOperator)
{
    EXPECT_TRUE(omath::Vector3<float>{} >= omath::Vector3<float>{});
    EXPECT_TRUE(omath::Vector3(1.f, 1.f, 1.f) >= omath::Vector3<float>{});
}

// Static assertions (compile-time checks)
static_assert(Vector3(1.0f, 2.0f, 3.0f).length_sqr() == 14.0f, "LengthSqr should be 14");
static_assert(Vector3(1.0f, 2.0f, 3.0f).dot(Vector3(4.0f, 5.0f, 6.0f)) == 32.0f, "Dot product should be 32");
static_assert(Vector3(4.0f, 5.0f, 6.0f).distance_to_sqr(Vector3(1.0f, 2.0f, 3.0f)) == 27.0f, "DistToSqr should be 27");
static_assert(Vector3(-1.0f, -2.0f, -3.0f).abs() == Vector3(1.0f, 2.0f, 3.0f), "Abs should convert negative values to positive");
