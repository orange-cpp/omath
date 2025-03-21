//
// Created by vlad on 9/24/2024.
//
//
// Vector4Test.cpp
//

#include <gtest/gtest.h>
#include <omath/Vector4.hpp>
#include <limits> // For std::numeric_limits

using namespace omath;

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
    constexpr float lengthSqr = Vector4(1.0f, 2.0f, 3.0f, 4.0f).LengthSqr();
    EXPECT_FLOAT_EQ(lengthSqr, 30.0f);
}

TEST_F(UnitTestVector4, DotProduct)
{
    constexpr float dot = Vector4(1.0f, 2.0f, 3.0f, 4.0f).Dot(Vector4(4.0f, 5.0f, 6.0f, 7.0f));
    EXPECT_FLOAT_EQ(dot, 60.0f);
}

TEST_F(UnitTestVector4, Abs)
{
    Vector4 v3 = Vector4(-1.0f, -2.0f, -3.0f, -4.0f);
    v3.Abs();
    EXPECT_FLOAT_EQ(v3.x, 1.0f);
    EXPECT_FLOAT_EQ(v3.y, 2.0f);
    EXPECT_FLOAT_EQ(v3.z, 3.0f);
    EXPECT_FLOAT_EQ(v3.w, 4.0f);
}

TEST_F(UnitTestVector4, Sum)
{
    constexpr float sum = Vector4(1.0f, 2.0f, 3.0f, 4.0f).Sum();
    EXPECT_FLOAT_EQ(sum, 10.0f);
}

TEST_F(UnitTestVector4, Clamp)
{
    Vector4 v3 = Vector4(1.0f, 2.0f, 3.0f, 4.0f);
    v3.Clamp(1.5f, 2.5f);
    EXPECT_FLOAT_EQ(v3.x, 1.5f);
    EXPECT_FLOAT_EQ(v3.y, 2.0f);
    EXPECT_FLOAT_EQ(v3.z, 2.5f);
    EXPECT_FLOAT_EQ(v3.w, 4.0f); // w is not clamped in this method
}
