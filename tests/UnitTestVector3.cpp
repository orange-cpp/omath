//
// Created by Vlad on 01.09.2024.
//
#include <gtest/gtest.h>
#include <omath/Vector3.h>

using namespace omath;

class UnitTestVector3 : public ::testing::Test
{
protected:
    Vector3 v1;
    Vector3 v2;

    void SetUp() override
    {
        v1 = Vector3(1.0f, 2.0f, 3.0f);
        v2 = Vector3(4.0f, 5.0f, 6.0f);
    }
};

// Test constructor and default values
TEST_F(UnitTestVector3, Constructor_Default)
{
    Vector3 v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST_F(UnitTestVector3, Constructor_Values)
{
    Vector3 v(1.0f, 2.0f, 3.0f);
    EXPECT_FLOAT_EQ(v.x, 1.0f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
    EXPECT_FLOAT_EQ(v.z, 3.0f);
}

// Test equality operators
TEST_F(UnitTestVector3, EqualityOperator)
{
    Vector3 v3(1.0f, 2.0f, 3.0f);
    EXPECT_TRUE(v1 == v3);
    EXPECT_FALSE(v1 == v2);
}

TEST_F(UnitTestVector3, InequalityOperator)
{
    Vector3 v3(1.0f, 2.0f, 3.0f);
    EXPECT_FALSE(v1 != v3);
    EXPECT_TRUE(v1 != v2);
}

// Test arithmetic operators
TEST_F(UnitTestVector3, AdditionOperator)
{
    Vector3 v3 = v1 + v2;
    EXPECT_FLOAT_EQ(v3.x, 5.0f);
    EXPECT_FLOAT_EQ(v3.y, 7.0f);
    EXPECT_FLOAT_EQ(v3.z, 9.0f);
}

TEST_F(UnitTestVector3, SubtractionOperator)
{
    Vector3 v3 = v2 - v1;
    EXPECT_FLOAT_EQ(v3.x, 3.0f);
    EXPECT_FLOAT_EQ(v3.y, 3.0f);
    EXPECT_FLOAT_EQ(v3.z, 3.0f);
}

TEST_F(UnitTestVector3, MultiplicationOperator)
{
    Vector3 v3 = v1 * 2.0f;
    EXPECT_FLOAT_EQ(v3.x, 2.0f);
    EXPECT_FLOAT_EQ(v3.y, 4.0f);
    EXPECT_FLOAT_EQ(v3.z, 6.0f);
}

TEST_F(UnitTestVector3, MultiplicationWithVectorOperator)
{
    Vector3 v3 = v1 * v2;
    EXPECT_FLOAT_EQ(v3.x, 4.0f);
    EXPECT_FLOAT_EQ(v3.y, 10.0f);
    EXPECT_FLOAT_EQ(v3.z, 18.0f);
}

TEST_F(UnitTestVector3, DivisionOperator)
{
    Vector3 v3 = v2 / 2.0f;
    EXPECT_FLOAT_EQ(v3.x, 2.0f);
    EXPECT_FLOAT_EQ(v3.y, 2.5f);
    EXPECT_FLOAT_EQ(v3.z, 3.0f);
}

TEST_F(UnitTestVector3, DivisionWithVectorOperator)
{
    Vector3 v3 = v2 / v1;
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
    Vector3 v3 = -v1;
    EXPECT_FLOAT_EQ(v3.x, -1.0f);
    EXPECT_FLOAT_EQ(v3.y, -2.0f);
    EXPECT_FLOAT_EQ(v3.z, -3.0f);
}

// Test other member functions
TEST_F(UnitTestVector3, DistTo)
{
    float dist = v1.DistTo(v2);
    EXPECT_FLOAT_EQ(dist, sqrt(27.0f));
}

TEST_F(UnitTestVector3, DistToSqr)
{
    float distSqr = v1.DistToSqr(v2);
    EXPECT_FLOAT_EQ(distSqr, 27.0f);
}

TEST_F(UnitTestVector3, DotProduct)
{
    float dot = v1.Dot(v2);
    EXPECT_FLOAT_EQ(dot, 32.0f);
}

TEST_F(UnitTestVector3, CrossProduct)
{
    Vector3 v3 = v1.Cross(v2);
    EXPECT_FLOAT_EQ(v3.x, -3.0f);
    EXPECT_FLOAT_EQ(v3.y, 6.0f);
    EXPECT_FLOAT_EQ(v3.z, -3.0f);
}

TEST_F(UnitTestVector3, Length)
{
    float length = v1.Length();
    EXPECT_FLOAT_EQ(length, sqrt(14.0f));
}

TEST_F(UnitTestVector3, LengthSqr)
{
    float lengthSqr = v1.LengthSqr();
    EXPECT_FLOAT_EQ(lengthSqr, 14.0f);
}

TEST_F(UnitTestVector3, Length2D)
{
    float length2D = v1.Length2D();
    EXPECT_FLOAT_EQ(length2D, sqrt(5.0f));
}

TEST_F(UnitTestVector3, Abs)
{
    Vector3 v3(-1.0f, -2.0f, -3.0f);
    v3.Abs();
    EXPECT_FLOAT_EQ(v3.x, 1.0f);
    EXPECT_FLOAT_EQ(v3.y, 2.0f);
    EXPECT_FLOAT_EQ(v3.z, 3.0f);
}

TEST_F(UnitTestVector3, Sum)
{
    float sum = v1.Sum();
    EXPECT_FLOAT_EQ(sum, 6.0f);
}

TEST_F(UnitTestVector3, Sum2D)
{
    float sum2D = v1.Sum2D();
    EXPECT_FLOAT_EQ(sum2D, 3.0f);
}

TEST_F(UnitTestVector3, ViewAngleTo)
{
    Vector3 angle = v1.ViewAngleTo(v1 + Vector3(0.f, 0.f, 5.f));
    EXPECT_NEAR(angle.x, 90.f, 0.01f);  // Approximate values, you can fine-tune the expected values
    EXPECT_NEAR(angle.y, 0.f, 0.01f);
}

TEST_F(UnitTestVector3, ForwardVector)
{
    Vector3 forward = Vector3::ForwardVector(0.0f, 0.0f);
    EXPECT_FLOAT_EQ(forward.x, 1.0f);
    EXPECT_FLOAT_EQ(forward.y, 0.0f);
    EXPECT_FLOAT_EQ(forward.z, 0.0f);
}

TEST_F(UnitTestVector3, RightVector)
{
    Vector3 right = Vector3::RightVector(0.0f, 0.0f, 0.0f);
    EXPECT_FLOAT_EQ(right.x, 0.0f);
    EXPECT_FLOAT_EQ(right.y, -1.0f);
    EXPECT_FLOAT_EQ(right.z, 0.0f);
}

TEST_F(UnitTestVector3, UpVector)
{
    Vector3 up = Vector3::UpVector(0.f, 0.0f, 0.0f);
    EXPECT_FLOAT_EQ(up.x, 0.0f);
    EXPECT_FLOAT_EQ(up.y, 0.0f);
    EXPECT_FLOAT_EQ(up.z, 1.0f);
}

TEST_F(UnitTestVector3, Normalized)
{
    Vector3 v3 = v1.Normalized();
    EXPECT_NEAR(v3.x, 0.26726f, 0.0001f);
    EXPECT_NEAR(v3.y, 0.53452f, 0.0001f);
    EXPECT_NEAR(v3.z, 0.80178f, 0.0001f);
}