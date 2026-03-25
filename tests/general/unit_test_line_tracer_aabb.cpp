//
// Created by Vlad on 3/25/2025.
//
#include "omath/collision/line_tracer.hpp"
#include "omath/linear_algebra/aabb.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <gtest/gtest.h>

using Vec3 = omath::Vector3<float>;
using Ray = omath::collision::Ray<>;
using LineTracer = omath::collision::LineTracer<>;
using AABB = omath::AABB<Vec3>;

static Ray make_ray(Vec3 start, Vec3 end, bool infinite = false)
{
    Ray r;
    r.start = start;
    r.end = end;
    r.infinite_length = infinite;
    return r;
}

// Ray passing straight through the center along Z axis
TEST(LineTracerAABBTests, HitCenterAlongZ)
{
    const AABB box{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    const auto ray = make_ray({0.f, 0.f, -5.f}, {0.f, 0.f, 5.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_NE(hit, ray.end);
    EXPECT_NEAR(hit.z, -1.f, 1e-4f);
    EXPECT_NEAR(hit.x, 0.f, 1e-4f);
    EXPECT_NEAR(hit.y, 0.f, 1e-4f);
}

// Ray passing straight through the center along X axis
TEST(LineTracerAABBTests, HitCenterAlongX)
{
    const AABB box{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    const auto ray = make_ray({-5.f, 0.f, 0.f}, {5.f, 0.f, 0.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_NE(hit, ray.end);
    EXPECT_NEAR(hit.x, -1.f, 1e-4f);
}

// Ray that misses entirely (too far in Y)
TEST(LineTracerAABBTests, MissReturnsEnd)
{
    const AABB box{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    const auto ray = make_ray({0.f, 5.f, -5.f}, {0.f, 5.f, 5.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_EQ(hit, ray.end);
}

// Ray that stops short before reaching the box
TEST(LineTracerAABBTests, RayTooShortReturnsEnd)
{
    const AABB box{{3.f, -1.f, -1.f}, {5.f, 1.f, 1.f}};
    const auto ray = make_ray({0.f, 0.f, 0.f}, {2.f, 0.f, 0.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_EQ(hit, ray.end);
}

// Infinite ray that starts before the box should hit
TEST(LineTracerAABBTests, InfiniteRayHits)
{
    const AABB box{{3.f, -1.f, -1.f}, {5.f, 1.f, 1.f}};
    const auto ray = make_ray({0.f, 0.f, 0.f}, {2.f, 0.f, 0.f}, true);

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_NE(hit, ray.end);
    EXPECT_NEAR(hit.x, 3.f, 1e-4f);
}

// Ray starting inside the box — t_min=0, so hit point equals ray.start
TEST(LineTracerAABBTests, RayStartsInsideBox)
{
    const AABB box{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    const auto ray = make_ray({0.f, 0.f, 0.f}, {0.f, 0.f, 5.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_NE(hit, ray.end);
    // t_min is clamped to 0, so hit == start
    EXPECT_NEAR(hit.x, 0.f, 1e-4f);
    EXPECT_NEAR(hit.y, 0.f, 1e-4f);
    EXPECT_NEAR(hit.z, 0.f, 1e-4f);
}

// Ray parallel to XY plane, pointing along X, at Z outside the box
TEST(LineTracerAABBTests, ParallelRayOutsideSlabMisses)
{
    const AABB box{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    // Z component of ray is 3.0 — outside box's Z slab
    const auto ray = make_ray({-5.f, 0.f, 3.f}, {5.f, 0.f, 3.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_EQ(hit, ray.end);
}

// Ray parallel to XY plane, pointing along X, at Z inside the box
TEST(LineTracerAABBTests, ParallelRayInsideSlabHits)
{
    const AABB box{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    const auto ray = make_ray({-5.f, 0.f, 0.f}, {5.f, 0.f, 0.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_NE(hit, ray.end);
    EXPECT_NEAR(hit.x, -1.f, 1e-4f);
}

// Diagonal ray hitting a corner region
TEST(LineTracerAABBTests, DiagonalRayHits)
{
    const AABB box{{0.f, 0.f, 0.f}, {2.f, 2.f, 2.f}};
    const auto ray = make_ray({-1.f, -1.f, -1.f}, {3.f, 3.f, 3.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_NE(hit, ray.end);
    // Entry point should be at (0,0,0)
    EXPECT_NEAR(hit.x, 0.f, 1e-4f);
    EXPECT_NEAR(hit.y, 0.f, 1e-4f);
    EXPECT_NEAR(hit.z, 0.f, 1e-4f);
}
