//
// Created by Vladislav on 07.05.2026.
//
#include "omath/3d_primitives/obb.hpp"
#include "omath/collision/line_tracer.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <numbers>

using Vec3 = omath::Vector3<float>;
using Ray = omath::collision::Ray<>;
using LineTracer = omath::collision::LineTracer<>;
using OBB = omath::primitives::Obb<float>;

namespace
{
    Ray make_ray(const Vec3 start, const Vec3 end, const bool infinite = false)
    {
        Ray r;
        r.start = start;
        r.end = end;
        r.infinite_length = infinite;
        return r;
    }

    constexpr OBB axis_aligned_obb(const Vec3& center, const Vec3& half_extents) noexcept
    {
        return OBB{center, {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}, half_extents};
    }

    OBB rotated_around_z(const Vec3& center, const Vec3& half_extents, const float radians) noexcept
    {
        const auto c = std::cos(radians);
        const auto s = std::sin(radians);
        return OBB{center, {c, s, 0.f}, {-s, c, 0.f}, {0.f, 0.f, 1.f}, half_extents};
    }
} // namespace

// --- axis-aligned OBB behaves like AABB ---

TEST(LineTracerOBBTests, AxisAlignedHitAlongZ)
{
    const auto box = axis_aligned_obb({0.f, 0.f, 0.f}, {1.f, 1.f, 1.f});
    const auto ray = make_ray({0.f, 0.f, -5.f}, {0.f, 0.f, 5.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_NE(hit, ray.end);
    EXPECT_NEAR(hit.x, 0.f, 1e-4f);
    EXPECT_NEAR(hit.y, 0.f, 1e-4f);
    EXPECT_NEAR(hit.z, -1.f, 1e-4f);
}

TEST(LineTracerOBBTests, AxisAlignedHitAlongX)
{
    const auto box = axis_aligned_obb({0.f, 0.f, 0.f}, {1.f, 1.f, 1.f});
    const auto ray = make_ray({-5.f, 0.f, 0.f}, {5.f, 0.f, 0.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_NE(hit, ray.end);
    EXPECT_NEAR(hit.x, -1.f, 1e-4f);
}

TEST(LineTracerOBBTests, MissReturnsEnd)
{
    const auto box = axis_aligned_obb({0.f, 0.f, 0.f}, {1.f, 1.f, 1.f});
    const auto ray = make_ray({0.f, 5.f, -5.f}, {0.f, 5.f, 5.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_EQ(hit, ray.end);
}

TEST(LineTracerOBBTests, RayTooShortReturnsEnd)
{
    const auto box = axis_aligned_obb({4.f, 0.f, 0.f}, {1.f, 1.f, 1.f});
    const auto ray = make_ray({0.f, 0.f, 0.f}, {2.f, 0.f, 0.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_EQ(hit, ray.end);
}

TEST(LineTracerOBBTests, InfiniteRayHits)
{
    const auto box = axis_aligned_obb({4.f, 0.f, 0.f}, {1.f, 1.f, 1.f});
    const auto ray = make_ray({0.f, 0.f, 0.f}, {2.f, 0.f, 0.f}, true);

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_NE(hit, ray.end);
    EXPECT_NEAR(hit.x, 3.f, 1e-4f);
}

TEST(LineTracerOBBTests, RayStartsInsideBox)
{
    const auto box = axis_aligned_obb({0.f, 0.f, 0.f}, {1.f, 1.f, 1.f});
    const auto ray = make_ray({0.f, 0.f, 0.f}, {5.f, 0.f, 0.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_NE(hit, ray.end);
    EXPECT_NEAR(hit.x, 0.f, 1e-4f);
    EXPECT_NEAR(hit.y, 0.f, 1e-4f);
    EXPECT_NEAR(hit.z, 0.f, 1e-4f);
}

TEST(LineTracerOBBTests, RayBehindBoxReturnsEnd)
{
    const auto box = axis_aligned_obb({4.f, 0.f, 0.f}, {1.f, 1.f, 1.f});
    const auto ray = make_ray({10.f, 0.f, 0.f}, {20.f, 0.f, 0.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_EQ(hit, ray.end);
}

// --- rotated OBB ---

TEST(LineTracerOBBTests, RotatedBoxHitOnRotatedFace)
{
    // Box centred at the origin, rotated 45° around Z. After rotation, the box's "near" face
    // (originally x=-1) is now perpendicular to the (1, 1, 0)/√2 direction. A ray approaching
    // from +X (along world -X) first hits the box at the rotated face — at x = √2 ≈ 1.414.
    const auto box = rotated_around_z({0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}, std::numbers::pi_v<float> / 4.f);
    const auto ray = make_ray({5.f, 0.f, 0.f}, {-5.f, 0.f, 0.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_NE(hit, ray.end);
    EXPECT_NEAR(hit.x, std::numbers::sqrt2_v<float>, 1e-4f);
    EXPECT_NEAR(hit.y, 0.f, 1e-4f);
    EXPECT_NEAR(hit.z, 0.f, 1e-4f);
}

TEST(LineTracerOBBTests, RotatedBoxMissesWhereAabbWouldHit)
{
    // A unit cube rotated 45° around Z has an XY footprint that is a diamond reaching
    // (±√2, 0) and (0, ±√2). The AABB envelope spans x,y ∈ [-√2, √2], but at y just below √2
    // the diamond is essentially a point. A ray at y = 1.43 is outside the diamond entirely
    // (|x| + |y| ≤ √2 ⇒ |x| ≤ √2 - 1.43 < 0), yet it would still pass through the AABB
    // envelope of the rotated box.
    const auto box = rotated_around_z({0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}, std::numbers::pi_v<float> / 4.f);
    const auto ray = make_ray({-5.f, 1.43f, 0.f}, {5.f, 1.43f, 0.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_EQ(hit, ray.end);
}

TEST(LineTracerOBBTests, RotatedThinBoxHitFromTheSide)
{
    // Long, thin axis-aligned slab along X, rotated 90° around Z so it now points along Y.
    // A ray from +X straight back along -X must miss (the slab is thin in X), but a ray along
    // -Y from +Y must hit.
    const auto box = rotated_around_z({0.f, 0.f, 0.f}, {5.f, 0.5f, 1.f}, std::numbers::pi_v<float> / 2.f);

    const auto ray_along_x = make_ray({10.f, 0.f, 0.f}, {-10.f, 0.f, 0.f});
    const auto hit_x = LineTracer::get_ray_hit_point(ray_along_x, box);
    EXPECT_NE(hit_x, ray_along_x.end);
    EXPECT_NEAR(hit_x.x, 0.5f, 1e-4f); // hit on the rotated slab's narrow side

    const auto ray_along_y = make_ray({0.f, 10.f, 0.f}, {0.f, -10.f, 0.f});
    const auto hit_y = LineTracer::get_ray_hit_point(ray_along_y, box);
    EXPECT_NE(hit_y, ray_along_y.end);
    EXPECT_NEAR(hit_y.y, 5.f, 1e-4f); // hit on the long end at y=+5
}

TEST(LineTracerOBBTests, RotatedAndTranslatedBoxHit)
{
    const auto box = rotated_around_z({10.f, 5.f, 0.f}, {1.f, 1.f, 1.f}, std::numbers::pi_v<float> / 4.f);
    // Ray approaches the rotated box from +X.
    const auto ray = make_ray({20.f, 5.f, 0.f}, {0.f, 5.f, 0.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_NE(hit, ray.end);
    EXPECT_NEAR(hit.x, 10.f + std::numbers::sqrt2_v<float>, 1e-4f);
    EXPECT_NEAR(hit.y, 5.f, 1e-4f);
}

TEST(LineTracerOBBTests, ParallelRayOutsideMisses)
{
    // Ray runs parallel to a slab face, completely outside the slab.
    const auto box = axis_aligned_obb({0.f, 0.f, 0.f}, {1.f, 1.f, 1.f});
    const auto ray = make_ray({-5.f, 2.f, 0.f}, {5.f, 2.f, 0.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_EQ(hit, ray.end);
}

TEST(LineTracerOBBTests, ParallelRayInsideHits)
{
    // Ray runs parallel to a slab face but inside the slab — should still hit the entry plane.
    const auto box = axis_aligned_obb({0.f, 0.f, 0.f}, {1.f, 1.f, 1.f});
    const auto ray = make_ray({-5.f, 0.5f, 0.f}, {5.f, 0.5f, 0.f});

    const auto hit = LineTracer::get_ray_hit_point(ray, box);
    EXPECT_NE(hit, ray.end);
    EXPECT_NEAR(hit.x, -1.f, 1e-4f);
}

TEST(LineTracerOBBTests, MatchesAabbForAxisAlignedBox)
{
    using AABB = omath::primitives::Aabb<float>;

    struct
    {
        Vec3 center;
        Vec3 half;
        Vec3 ray_start;
        Vec3 ray_end;
    } cases[] = {
            {{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}, {-5.f, 0.f, 0.f}, {5.f, 0.f, 0.f}},
            {{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}, {0.f, -5.f, 0.f}, {0.f, 5.f, 0.f}},
            {{4.f, 0.f, 0.f}, {1.f, 1.f, 1.f}, {0.f, 0.f, 0.f}, {2.f, 0.f, 0.f}}, // too short
            {{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}, {-5.f, 5.f, 0.f}, {5.f, 5.f, 0.f}}, // miss
            {{2.f, 3.f, -1.f}, {0.5f, 0.5f, 0.5f}, {0.f, 0.f, 0.f}, {10.f, 15.f, -5.f}}, // diagonal
    };

    for (const auto& [center, half, start, end]: cases)
    {
        const AABB aabb{center - half, center + half};
        const auto obb = axis_aligned_obb(center, half);
        const auto ray = make_ray(start, end);

        const auto aabb_hit = LineTracer::get_ray_hit_point(ray, aabb);
        const auto obb_hit = LineTracer::get_ray_hit_point(ray, obb);

        EXPECT_NEAR(aabb_hit.x, obb_hit.x, 1e-4f);
        EXPECT_NEAR(aabb_hit.y, obb_hit.y, 1e-4f);
        EXPECT_NEAR(aabb_hit.z, obb_hit.z, 1e-4f);
    }
}
