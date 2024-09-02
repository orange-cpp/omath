//
// Created by Vlad on 02.09.2024.
//
//
// Created by Vlad on 01.09.2024.
//
#include <gtest/gtest.h>
#include <omath/Vector2.h>

using namespace omath;

class UnitTestVector2 : public ::testing::Test
{
protected:
    Vector2 v1;
    Vector2 v2;

    void SetUp() override
    {
        v1 = Vector2(1.0f, 2.0f);
        v2 = Vector2(4.0f, 5.0f);
    }
};

// Test constructor and default values
TEST_F(UnitTestVector2, Constructor_Default)
{
    Vector2 v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
}

TEST_F(UnitTestVector2, Constructor_Values)
{
    Vector2 v(1.0f, 2.0f);
    EXPECT_FLOAT_EQ(v.x, 1.0f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
}

// Test equality operators
TEST_F(UnitTestVector2, EqualityOperator)
{
    Vector2 v3(1.0f, 2.0f);
    EXPECT_TRUE(v1 == v3);
    EXPECT_FALSE(v1 == v2);
}

TEST_F(UnitTestVector2, InequalityOperator)
{
    Vector2 v3(1.0f, 2.0f);
    EXPECT_FALSE(v1 != v3);
    EXPECT_TRUE(v1 != v2);
}

// Test arithmetic operators
TEST_F(UnitTestVector2, AdditionOperator)
{
    Vector2 v3 = v1 + v2;
    EXPECT_FLOAT_EQ(v3.x, 5.0f);
    EXPECT_FLOAT_EQ(v3.y, 7.0f);
}

TEST_F(UnitTestVector2, SubtractionOperator)
{
    Vector2 v3 = v2 - v1;
    EXPECT_FLOAT_EQ(v3.x, 3.0f);
    EXPECT_FLOAT_EQ(v3.y, 3.0f);
}

TEST_F(UnitTestVector2, MultiplicationOperator)
{
    Vector2 v3 = v1 * 2.0f;
    EXPECT_FLOAT_EQ(v3.x, 2.0f);
    EXPECT_FLOAT_EQ(v3.y, 4.0f);
}

TEST_F(UnitTestVector2, DivisionOperator)
{
    Vector2 v3 = v2 / 2.0f;
    EXPECT_FLOAT_EQ(v3.x, 2.0f);
    EXPECT_FLOAT_EQ(v3.y, 2.5f);
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

TEST_F(UnitTestVector2, NegationOperator)
{
    Vector2 v3 = -v1;
    EXPECT_FLOAT_EQ(v3.x, -1.0f);
    EXPECT_FLOAT_EQ(v3.y, -2.0f);
}

// Test other member functions
TEST_F(UnitTestVector2, DistTo)
{
    float dist = v1.DistTo(v2);
    EXPECT_FLOAT_EQ(dist, sqrt(18.0f));
}

TEST_F(UnitTestVector2, DistToSqr)
{
    float distSqr = v1.DistToSqr(v2);
    EXPECT_FLOAT_EQ(distSqr, 18.0f);
}

TEST_F(UnitTestVector2, DotProduct)
{
    float dot = v1.Dot(v2);
    EXPECT_FLOAT_EQ(dot, 14.0f);
}

TEST_F(UnitTestVector2, Length)
{
    float length = v1.Length();
    EXPECT_FLOAT_EQ(length, sqrt(5.0f));
}

TEST_F(UnitTestVector2, LengthSqr)
{
    float lengthSqr = v1.LengthSqr();
    EXPECT_FLOAT_EQ(lengthSqr, 5.0f);
}

TEST_F(UnitTestVector2, Abs)
{
    Vector2 v3(-1.0f, -2.0f);
    v3.Abs();
    EXPECT_FLOAT_EQ(v3.x, 1.0f);
    EXPECT_FLOAT_EQ(v3.y, 2.0f);
}

TEST_F(UnitTestVector2, Sum)
{
    float sum = v1.Sum();
    EXPECT_FLOAT_EQ(sum, 3.0f);
}

TEST_F(UnitTestVector2, Normalized)
{
    Vector2 v3 = v1.Normalized();
    EXPECT_NEAR(v3.x, 0.44721f, 0.0001f);
    EXPECT_NEAR(v3.y, 0.89443f, 0.0001f);
}
