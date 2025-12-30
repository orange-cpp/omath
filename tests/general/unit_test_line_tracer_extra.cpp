// Extra LineTracer tests
#include <gtest/gtest.h>
#include <omath/collision/line_tracer.hpp>
#include <omath/linear_algebra/vector3.hpp>

using namespace omath;
using namespace omath::collision;

TEST(LineTracerExtra, MissParallel)
{
    constexpr Triangle<Vector3<float>> tri({0,0,0},{1,0,0},{0,1,0});
    constexpr Ray ray{ .start=.start={0.3f,0.3f,1.f}, .end=.end={0.3f,0.3f,2.f}, .infinite_length=.infinite_length=false }; // parallel above triangle
    const auto hit = LineTracer::get_ray_hit_point(ray, tri);
    EXPECT_EQ(hit, ray.end);
}

TEST(LineTracerExtra, HitCenter)
{
    constexpr Triangle<Vector3<float>> tri({0,0,0},{1,0,0},{0,1,0});
    constexpr Ray ray{ .start=.start={0.3f,0.3f,-1.f}, .end=.end={0.3f,0.3f,1.f}, .infinite_length=.infinite_length=false };
    const auto hit = LineTracer::get_ray_hit_point(ray, tri);
    ASSERT_FALSE(hit == ray.end);
    EXPECT_NEAR(hit.x, 0.3f, 1e-6f);
    EXPECT_NEAR(hit.y, 0.3f, 1e-6f);
    EXPECT_NEAR(hit.z, 0.f, 1e-6f);
}

TEST(LineTracerExtra, HitOnEdge)
{
    constexpr Triangle<Vector3<float>> tri({0,0,0},{1,0,0},{0,1,0});
    constexpr Ray ray{ .start=.start={0.0f,0.0f,1.f}, .end=.end={0.0f,0.0f,0.f}, .infinite_length=.infinite_length=false };
    // hitting exact vertex/edge may be considered miss; ensure function handles without crash
    if (const auto hit = LineTracer::get_ray_hit_point(ray, tri); hit != ray.end)
    {
        EXPECT_NEAR(hit.x, 0.0f, 1e-6f);
        EXPECT_NEAR(hit.y, 0.0f, 1e-6f);
    }
}

TEST(LineTracerExtra, InfiniteRayIgnoredIfBehind)
{
    constexpr Triangle<Vector3<float>> tri({0,0,0},{1,0,0},{0,1,0});
    // Ray pointing away but infinite_length true should be ignored
    constexpr Ray ray{ .start=.start={0.5f,0.5f,-1.f}, .end=.end={0.5f,0.5f,-2.f}, .infinite_length=.infinite_length=true };
    const auto hit = LineTracer::get_ray_hit_point(ray, tri);
    EXPECT_EQ(hit, ray.end);
}
