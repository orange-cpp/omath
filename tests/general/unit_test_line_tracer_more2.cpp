#include "omath/collision/line_tracer.hpp"
#include "omath/linear_algebra/triangle.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <gtest/gtest.h>

using omath::Vector3;
using omath::collision::Ray;
using omath::collision::LineTracer;
using Triangle3 = omath::Triangle<Vector3<float>>;

TEST(LineTracerMore2, UGreaterThanOneReturnsEnd)
{
    Triangle3 tri({0.f,0.f,0.f},{1.f,0.f,0.f},{0.f,1.f,0.f});
    // choose ray so barycentric u > 1
    Ray ray; ray.start = {2.f, -1.f, -1.f}; ray.end = {2.f, -1.f, 1.f};

    auto hit = LineTracer::get_ray_hit_point(ray, tri);
    EXPECT_EQ(hit, ray.end);
}

TEST(LineTracerMore2, VGreaterThanOneReturnsEnd)
{
    Triangle3 tri({0.f,0.f,0.f},{1.f,0.f,0.f},{0.f,1.f,0.f});
    // choose ray so barycentric v > 1
    Ray ray; ray.start = {-1.f, 2.f, -1.f}; ray.end = {-1.f, 2.f, 1.f};

    auto hit = LineTracer::get_ray_hit_point(ray, tri);
    EXPECT_EQ(hit, ray.end);
}

TEST(LineTracerMore2, UPlusVGreaterThanOneReturnsEnd)
{
    Triangle3 tri({0.f,0.f,0.f},{1.f,0.f,0.f},{0.f,1.f,0.f});
    // Ray aimed so u+v > 1 (outside triangle region)
    Ray ray; ray.start = {1.f, 1.f, -1.f}; ray.end = {1.f, 1.f, 1.f};

    auto hit = LineTracer::get_ray_hit_point(ray, tri);
    EXPECT_EQ(hit, ray.end);
}

TEST(LineTracerMore2, DirectionVectorNormalizedProducesUnitLength)
{
    Ray r; r.start = {0.f,0.f,0.f}; r.end = {0.f,3.f,4.f};
    auto dir = r.direction_vector_normalized();
    auto len = dir.length();
    EXPECT_NEAR(len, 1.f, 1e-6f);
}

TEST(LineTracerMore2, ZeroLengthRayHandled)
{
    Triangle3 tri({0.f,0.f,0.f},{1.f,0.f,0.f},{0.f,1.f,0.f});
    Ray ray; ray.start = {0.f,0.f,0.f}; ray.end = {0.f,0.f,0.f};

    // Zero-length ray: direction length == 0; algorithm should handle without crash
    auto hit = LineTracer::get_ray_hit_point(ray, tri);
    EXPECT_EQ(hit, ray.end);
}
