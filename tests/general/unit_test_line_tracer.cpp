#include "omath/collision/line_tracer.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include "omath/linear_algebra/triangle.hpp"
#include <gtest/gtest.h>

using omath::Vector3;

TEST(LineTracerTests, ParallelRayReturnsEnd)
{
    // Triangle in XY plane
    omath::Triangle<Vector3<float>> tri{ {0.f,0.f,0.f}, {1.f,0.f,0.f}, {0.f,1.f,0.f} };
    omath::collision::Ray ray;
    ray.start = Vector3<float>{0.f,0.f,1.f};
    ray.end = Vector3<float>{1.f,1.f,2.f}; // direction parallel to plane normal (z) -> but choose parallel to plane? make direction parallel to triangle plane
    ray.end = Vector3<float>{1.f,1.f,1.f};

    // For a ray parallel to the triangle plane the algorithm should return ray.end
    auto hit = omath::collision::LineTracer::get_ray_hit_point(ray, tri);
    EXPECT_TRUE(hit == ray.end);
    EXPECT_TRUE(omath::collision::LineTracer::can_trace_line(ray, tri));
}

TEST(LineTracerTests, MissesTriangleReturnsEnd)
{
    omath::Triangle<Vector3<float>> tri{ {0.f,0.f,0.f}, {1.f,0.f,0.f}, {0.f,1.f,0.f} };
    omath::collision::Ray ray;
    ray.start = Vector3<float>{2.f,2.f,-1.f};
    ray.end = Vector3<float>{2.f,2.f,1.f}; // passes above the triangle area

    auto hit = omath::collision::LineTracer::get_ray_hit_point(ray, tri);
    EXPECT_TRUE(hit == ray.end);
}

TEST(LineTracerTests, HitTriangleReturnsPointInsideSegment)
{
    omath::Triangle<Vector3<float>> tri{ {0.f,0.f,0.f}, {2.f,0.f,0.f}, {0.f,2.f,0.f} };
    omath::collision::Ray ray;
    ray.start = Vector3<float>{0.25f,0.25f,-1.f};
    ray.end = Vector3<float>{0.25f,0.25f,1.f};

    auto hit = omath::collision::LineTracer::get_ray_hit_point(ray, tri);
    // Should return a point between start and end (z approximately 0)
    EXPECT_NE(hit, ray.end);
    EXPECT_NEAR(hit.z, 0.f, 1e-4f);
    // t_hit should be between 0 and 1 along the ray direction
    auto dir = ray.direction_vector();
    // find t such that start + dir * t == hit (only check z comp for stability)
    float t = (hit.z - ray.start.z) / dir.z;
    EXPECT_GT(t, 0.f);
    EXPECT_LT(t, 1.f);
}

TEST(LineTracerTests, InfiniteLengthEarlyOut)
{
    omath::Triangle<Vector3<float>> tri{ {0.f,0.f,0.f}, {1.f,0.f,0.f}, {0.f,1.f,0.f} };
    omath::collision::Ray ray;
    ray.start = Vector3<float>{0.25f,0.25f,0.f};
    ray.end = Vector3<float>{0.25f,0.25f,1.f};
    ray.infinite_length = true;

    // If t_hit <= epsilon the algorithm should return ray.end when infinite_length is true.
    // Using start on the triangle plane should produce t_hit <= epsilon.
    auto hit = omath::collision::LineTracer::get_ray_hit_point(ray, tri);
    EXPECT_TRUE(hit == ray.end);
}
