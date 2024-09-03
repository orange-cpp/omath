//
// Created by Vlad on 01.09.2024.
//
#include <gtest/gtest.h>
#include <omath/Vector3.h>
#include <cmath>

using namespace omath;

class UnitTestVector3 : public ::testing::Test
{
protected:
    Vector3 v1;
    Vector3 v2;

    constexpr void SetUp() override
    {
        v1 = Vector3(1.0f, 2.0f, 3.0f);
        v2 = Vector3(4.0f, 5.0f, 6.0f);
    }
};

// Test constructor and default values
TEST_F(UnitTestVector3, Constructor_Default)
{
    constexpr Vector3 v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST_F(UnitTestVector3, Constructor_Values)
{
    constexpr Vector3 v(1.0f, 2.0f, 3.0f);
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

TEST_F(UnitTestVector3, MultiplicationOperator)
{
    constexpr Vector3 v3 = Vector3(1.0f, 2.0f, 3.0f) * 2.0f;
    EXPECT_FLOAT_EQ(v3.x, 2.0f);
    EXPECT_FLOAT_EQ(v3.y, 4.0f);
    EXPECT_FLOAT_EQ(v3.z, 6.0f);
}

TEST_F(UnitTestVector3, MultiplicationWithVectorOperator)
{
    constexpr Vector3 v3 = Vector3(1.0f, 2.0f, 3.0f) * Vector3(4.0f, 5.0f, 6.0f);
    EXPECT_FLOAT_EQ(v3.x, 4.0f);
    EXPECT_FLOAT_EQ(v3.y, 10.0f);
    EXPECT_FLOAT_EQ(v3.z, 18.0f);
}

TEST_F(UnitTestVector3, DivisionOperator)
{
    constexpr Vector3 v3 = Vector3(4.0f, 5.0f, 6.0f) / 2.0f;
    EXPECT_FLOAT_EQ(v3.x, 2.0f);
    EXPECT_FLOAT_EQ(v3.y, 2.5f);
    EXPECT_FLOAT_EQ(v3.z, 3.0f);
}

TEST_F(UnitTestVector3, DivisionWithVectorOperator)
{
    constexpr Vector3 v3 = Vector3(4.0f, 5.0f, 6.0f) / Vector3(1.0f, 2.0f, 3.0f);
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

TEST_F(UnitTestVector3, MultiplicationAssignmentOperator)
{
    v1 *= 2.0f;
    EXPECT_FLOAT_EQ(v1.x, 2.0f);
    EXPECT_FLOAT_EQ(v1.y, 4.0f);
    EXPECT_FLOAT_EQ(v1.z, 6.0f);
}

TEST_F(UnitTestVector3, MultiplicationWithVectorAssignmentOperator)
{
    v1 *= v2;
    EXPECT_FLOAT_EQ(v1.x, 4.0f);
    EXPECT_FLOAT_EQ(v1.y, 10.0f);
    EXPECT_FLOAT_EQ(v1.z, 18.0f);
}

TEST_F(UnitTestVector3, DivisionAssignmentOperator)
{
    v1 /= 2.0f;
    EXPECT_FLOAT_EQ(v1.x, 0.5f);
    EXPECT_FLOAT_EQ(v1.y, 1.0f);
    EXPECT_FLOAT_EQ(v1.z, 1.5f);
}

TEST_F(UnitTestVector3, DivisionWithVectorAssignmentOperator)
{
    v1 /= v2;
    EXPECT_FLOAT_EQ(v1.x, 0.25f);
    EXPECT_FLOAT_EQ(v1.y, 0.4f);
    EXPECT_FLOAT_EQ(v1.z, 0.5f);
}

TEST_F(UnitTestVector3, NegationOperator)
{
    constexpr Vector3 v3 = -Vector3(1.0f, 2.0f, 3.0f);
    EXPECT_FLOAT_EQ(v3.x, -1.0f);
    EXPECT_FLOAT_EQ(v3.y, -2.0f);
    EXPECT_FLOAT_EQ(v3.z, -3.0f);
}

// Test other member functions
TEST_F(UnitTestVector3, DistToSqr)
{
    constexpr float distSqr = Vector3(1.0f, 2.0f, 3.0f).DistToSqr(Vector3(4.0f, 5.0f, 6.0f));
    EXPECT_FLOAT_EQ(distSqr, 27.0f);
}

TEST_F(UnitTestVector3, DotProduct)
{
    constexpr float dot = Vector3(1.0f, 2.0f, 3.0f).Dot(Vector3(4.0f, 5.0f, 6.0f));
    EXPECT_FLOAT_EQ(dot, 32.0f);
}

TEST_F(UnitTestVector3, LengthSqr)
{
    constexpr float lengthSqr = Vector3(1.0f, 2.0f, 3.0f).LengthSqr();
    EXPECT_FLOAT_EQ(lengthSqr, 14.0f);
}

TEST_F(UnitTestVector3, Abs)
{
    constexpr Vector3 v3 = Vector3(-1.0f, -2.0f, -3.0f).Abs();
    EXPECT_FLOAT_EQ(v3.x, 1.0f);
    EXPECT_FLOAT_EQ(v3.y, 2.0f);
    EXPECT_FLOAT_EQ(v3.z, 3.0f);
}

TEST_F(UnitTestVector3, Sum)
{
    constexpr float sum = Vector3(1.0f, 2.0f, 3.0f).Sum();
    EXPECT_FLOAT_EQ(sum, 6.0f);
}

TEST_F(UnitTestVector3, Sum2D)
{
    constexpr float sum2D = Vector3(1.0f, 2.0f, 3.0f).Sum2D();
    EXPECT_FLOAT_EQ(sum2D, 3.0f);
}

TEST_F(UnitTestVector3, CrossProduct)
{
    constexpr Vector3 v3 = Vector3(1.0f, 2.0f, 3.0f).Cross(Vector3(4.0f, 5.0f, 6.0f));
    EXPECT_FLOAT_EQ(v3.x, -3.0f);
    EXPECT_FLOAT_EQ(v3.y, 6.0f);
    EXPECT_FLOAT_EQ(v3.z, -3.0f);
}

// Test constexpr with static_assert
static_assert(Vector3(1.0f, 2.0f, 3.0f).LengthSqr() == 14.0f, "LengthSqr should be 14");
static_assert(Vector3(1.0f, 2.0f, 3.0f).Dot(Vector3(4.0f, 5.0f, 6.0f)) == 32.0f, "Dot product should be 32");
static_assert(Vector3(4.0f, 5.0f, 6.0f).DistToSqr(Vector3(1.0f, 2.0f, 3.0f)) == 27.0f, "DistToSqr should be 27");
static_assert(Vector3(-1.0f, -2.0f, -3.0f).Abs() == Vector3(1.0f, 2.0f, 3.0f), "Abs should convert negative values to positive");
