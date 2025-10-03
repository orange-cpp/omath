//
// Created by Orange on 1/6/2025.
//
#include "omath/linear_algebra/triangle.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <cmath> // For std::sqrt, std::isinf, std::isnan
#include <gtest/gtest.h>

using namespace omath;

class UnitTestTriangle : public ::testing::Test
{
protected:
    // Define some Triangles to use in tests
    Triangle<Vector3<float>> t1;
    Triangle<Vector3<float>> t2;
    Triangle<Vector3<float>> t3;

    constexpr void SetUp() override
    {
        // Triangle with vertices (0, 0, 0), (1, 0, 0), (0, 1, 0)
        t1 = Triangle<Vector3<float>>(
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(1.0f, 0.0f, 0.0f),
            Vector3(0.0f, 1.0f, 0.0f)
        );

        // Triangle with vertices (1, 2, 3), (4, 5, 6), (7, 8, 9)
        t2 = Triangle<Vector3<float>>(
            Vector3(1.0f, 2.0f, 3.0f),
            Vector3(4.0f, 5.0f, 6.0f),
            Vector3(7.0f, 8.0f, 9.0f)
        );

        // An isosceles right triangle
        t3 = Triangle<Vector3<float>>(
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(2.0f, 0.0f, 0.0f),
            Vector3(0.0f, 2.0f, 0.0f)
        );
    }
};

// Test constructor and vertices
TEST_F(UnitTestTriangle, Constructor)
{
    constexpr Triangle<Vector3<float>> t(
        Vector3(1.0f, 2.0f, 3.0f),
        Vector3(4.0f, 5.0f, 6.0f),
        Vector3(7.0f, 8.0f, 9.0f)
    );

    EXPECT_FLOAT_EQ(t.m_vertex1.x, 1.0f);
    EXPECT_FLOAT_EQ(t.m_vertex1.y, 2.0f);
    EXPECT_FLOAT_EQ(t.m_vertex1.z, 3.0f);

    EXPECT_FLOAT_EQ(t.m_vertex2.x, 4.0f);
    EXPECT_FLOAT_EQ(t.m_vertex2.y, 5.0f);
    EXPECT_FLOAT_EQ(t.m_vertex2.z, 6.0f);

    EXPECT_FLOAT_EQ(t.m_vertex3.x, 7.0f);
    EXPECT_FLOAT_EQ(t.m_vertex3.y, 8.0f);
    EXPECT_FLOAT_EQ(t.m_vertex3.z, 9.0f);
}

// Test CalculateNormal
TEST_F(UnitTestTriangle, CalculateNormal)
{
    // For t1, the normal should point in the +Z direction (0, 0, 1) or (0, 0, -1)
    const Vector3 normal_t1 = t1.calculate_normal();
    // Check if it's normalized and pointed along Z (sign can differ, so use absolute check)
    EXPECT_NEAR(std::fabs(normal_t1.z), 1.0f, 1e-5f);
    EXPECT_NEAR(normal_t1.length(), 1.0f, 1e-5f);


    // For t3, we expect the normal to be along +Z as well
    const Vector3 normal_t3 = t3.calculate_normal();
    EXPECT_NEAR(std::fabs(normal_t3.z), 1.0f, 1e-5f);
}

// Test side lengths
TEST_F(UnitTestTriangle, SideLengths)
{
    // For t1 side lengths
    EXPECT_FLOAT_EQ(t1.side_a_length(), std::sqrt(1.0f)); // distance between (0,0,0) and (1,0,0)
    EXPECT_FLOAT_EQ(t1.side_b_length(), std::sqrt(1.0f + 1.0f)); // distance between (4,5,6) & (7,8,9)... but we are testing t1, so let's be accurate:
    // Actually, for t1: vertex2=(1,0,0), vertex3=(0,1,0)
    // Dist between (0,1,0) and (1,0,0) = sqrt((1-0)^2 + (0-1)^2) = sqrt(1 + 1) = sqrt(2)
    EXPECT_FLOAT_EQ(t1.side_b_length(), std::sqrt(2.0f));

    // For t3, side a = distance between vertex1=(0,0,0) and vertex2=(2,0,0), which is 2
    // side b = distance between vertex3=(0,2,0) and vertex2=(2,0,0), which is sqrt(2^2 + (-2)^2)= sqrt(8)= 2.828...
    // We'll just check side a first:
    EXPECT_FLOAT_EQ(t3.side_a_length(), 2.0f);
    // Then side b:
    EXPECT_FLOAT_EQ(t3.side_b_length(), std::sqrt(8.0f));
}

// Test side vectors
TEST_F(UnitTestTriangle, SideVectors)
{
    const Vector3 sideA_t1 = t1.side_a_vector(); // m_vertex1 - m_vertex2
    EXPECT_FLOAT_EQ(sideA_t1.x, 0.0f - 1.0f);
    EXPECT_FLOAT_EQ(sideA_t1.y, 0.0f - 0.0f);
    EXPECT_FLOAT_EQ(sideA_t1.z, 0.0f - 0.0f);

    const Vector3 sideB_t1 = t1.side_b_vector(); // m_vertex3 - m_vertex2
    EXPECT_FLOAT_EQ(sideB_t1.x, 0.0f - 1.0f);
    EXPECT_FLOAT_EQ(sideB_t1.y, 1.0f - 0.0f);
    EXPECT_FLOAT_EQ(sideB_t1.z, 0.0f - 0.0f);
}

TEST_F(UnitTestTriangle, IsRectangular)
{
    EXPECT_TRUE(Triangle<Vector3<float>>({2,0,0}, {}, {0,2,0}).is_rectangular());
}
// Test midpoint
TEST_F(UnitTestTriangle, MidPoint)
{
    // For t1, midpoint of (0,0,0), (1,0,0), (0,1,0)
    const Vector3 mid1 = t1.mid_point();
    EXPECT_FLOAT_EQ(mid1.x, (0.0f + 1.0f + 0.0f) / 3.0f);
    EXPECT_FLOAT_EQ(mid1.y, (0.0f + 0.0f + 1.0f) / 3.0f);
    EXPECT_FLOAT_EQ(mid1.z, 0.0f);

    // For t2, midpoint of (1,2,3), (4,5,6), (7,8,9)
    const Vector3 mid2 = t2.mid_point();
    EXPECT_FLOAT_EQ(mid2.x, (1.0f + 4.0f + 7.0f) / 3.0f);
    EXPECT_FLOAT_EQ(mid2.y, (2.0f + 5.0f + 8.0f) / 3.0f);
    EXPECT_FLOAT_EQ(mid2.z, (3.0f + 6.0f + 9.0f) / 3.0f);
}
