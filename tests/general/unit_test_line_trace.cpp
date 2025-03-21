#include "gtest/gtest.h"
#include "omath/collision/line_tracer.hpp"
#include "omath/Triangle.hpp"
#include "omath/Vector3.hpp"

using namespace omath;
using namespace omath::collision;

class LineTracerTest : public ::testing::Test
{
protected:
    // Set up common variables for use in each test
    Vector3<float> vertex1{0.0f, 0.0f, 0.0f};
    Vector3<float> vertex2{1.0f, 0.0f, 0.0f};
    Vector3<float> vertex3{0.0f, 1.0f, 0.0f};
    Triangle<Vector3<float>> triangle{vertex1, vertex2, vertex3};
};

// Test that a ray intersecting the triangle returns false for CanTraceLine
TEST_F(LineTracerTest, RayIntersectsTriangle)
{
    constexpr Ray ray{{0.3f, 0.3f, -1.0f}, {0.3f, 0.3f, 1.0f}};
    EXPECT_FALSE(LineTracer::CanTraceLine(ray, triangle));
}

// Test that a ray parallel to the triangle plane returns true for CanTraceLine
TEST_F(LineTracerTest, RayParallelToTriangle)
{
    constexpr Ray ray{{0.3f, 0.3f, 1.0f}, {0.3f, 0.3f, 2.0f}};
    EXPECT_TRUE(LineTracer::CanTraceLine(ray, triangle));
}

// Test that a ray starting inside the triangle but pointing away returns true
TEST_F(LineTracerTest, RayStartsInTriangleButDoesNotIntersect)
{
    constexpr Ray ray{{0.3f, 0.3f, 0.0f}, {0.3f, 0.3f, -1.0f}};
    EXPECT_TRUE(LineTracer::CanTraceLine(ray, triangle));
}

// Test that a ray not intersecting the triangle plane returns true
TEST_F(LineTracerTest, RayMissesTriangle)
{
    constexpr Ray ray{{2.0f, 2.0f, -1.0f}, {2.0f, 2.0f, 1.0f}};
    EXPECT_TRUE(LineTracer::CanTraceLine(ray, triangle));
}

// Test that a ray lying exactly in the plane of the triangle without intersecting returns true
TEST_F(LineTracerTest, RayInPlaneNotIntersecting)
{
    constexpr Ray ray{{-1.0f, -1.0f, 0.0f}, {1.5f, 1.5f, 0.0f}};
    EXPECT_TRUE(LineTracer::CanTraceLine(ray, triangle));
}


TEST_F(LineTracerTest, RayIntersectsVertex)
{
    const Ray ray{{-1.0f, -1.0f, -1.0f}, vertex1}; // Intersecting at vertex1
    EXPECT_TRUE(LineTracer::CanTraceLine(ray, triangle));
}

TEST_F(LineTracerTest, RayIntersectsEdge)
{
    constexpr Ray ray{{-1.0f, 0.0f, -1.0f}, {0.5f, 0.0f, 0.0f}};
    // Intersecting on the edge between vertex1 and vertex2
    EXPECT_TRUE(LineTracer::CanTraceLine(ray, triangle));
}

TEST_F(LineTracerTest, TriangleFarBeyondRayEndPoint)
{
    // Define a ray with a short length
    constexpr Ray ray{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};

    // Define a triangle far beyond the ray's endpoint
    constexpr Triangle<Vector3<float>> distantTriangle{
        {1000.0f, 1000.0f, 1000.0f}, {1001.0f, 1000.0f, 1000.0f}, {1000.0f, 1001.0f, 1000.0f}
    };

    // Expect true because the ray ends long before it could reach the distant triangle
    EXPECT_TRUE(LineTracer::CanTraceLine(ray, distantTriangle));
}
