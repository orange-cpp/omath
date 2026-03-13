//
// Created by orange-cpp
//
#ifdef OMATH_ENABLE_PHYSX

#include <gtest/gtest.h>
#include <omath/collision/gjk_algorithm.hpp>
#include <omath/collision/physx_box_collider.hpp>
#include <omath/collision/physx_sphere_collider.hpp>

using namespace omath::collision;
using omath::Vector3;

// ─── PhysXBoxCollider ────────────────────────────────────────────────────────

TEST(PhysXBoxCollider, DefaultOriginIsZero)
{
    PhysXBoxCollider box({1.f, 1.f, 1.f});
    EXPECT_EQ(box.get_origin(), Vector3<float>(0.f, 0.f, 0.f));
}

TEST(PhysXBoxCollider, SetAndGetOrigin)
{
    PhysXBoxCollider box({1.f, 1.f, 1.f}, {3.f, 4.f, 5.f});
    EXPECT_EQ(box.get_origin(), Vector3<float>(3.f, 4.f, 5.f));

    box.set_origin({-1.f, 0.f, 2.f});
    EXPECT_EQ(box.get_origin(), Vector3<float>(-1.f, 0.f, 2.f));
}

TEST(PhysXBoxCollider, FurthestPointPositiveDirection)
{
    // Box centred at origin with half-extents (2, 3, 4).
    // Direction (+x, +y, +z) → furthest corner is (+2, +3, +4).
    PhysXBoxCollider box({2.f, 3.f, 4.f});
    const auto p = box.find_abs_furthest_vertex_position({1.f, 1.f, 1.f});
    EXPECT_FLOAT_EQ(p.x, 2.f);
    EXPECT_FLOAT_EQ(p.y, 3.f);
    EXPECT_FLOAT_EQ(p.z, 4.f);
}

TEST(PhysXBoxCollider, FurthestPointNegativeDirection)
{
    // Direction (-x, -y, -z) → furthest corner is (-2, -3, -4).
    PhysXBoxCollider box({2.f, 3.f, 4.f});
    const auto p = box.find_abs_furthest_vertex_position({-1.f, -1.f, -1.f});
    EXPECT_FLOAT_EQ(p.x, -2.f);
    EXPECT_FLOAT_EQ(p.y, -3.f);
    EXPECT_FLOAT_EQ(p.z, -4.f);
}

TEST(PhysXBoxCollider, FurthestPointMixedDirection)
{
    // Direction (+x, -y, +z) → furthest corner is (+2, -3, +4).
    PhysXBoxCollider box({2.f, 3.f, 4.f});
    const auto p = box.find_abs_furthest_vertex_position({1.f, -1.f, 1.f});
    EXPECT_FLOAT_EQ(p.x,  2.f);
    EXPECT_FLOAT_EQ(p.y, -3.f);
    EXPECT_FLOAT_EQ(p.z,  4.f);
}

TEST(PhysXBoxCollider, FurthestPointWithNonZeroOrigin)
{
    // Box at (10, 0, 0), half-extents (1, 1, 1). Direction +x → (11, 1, 1).
    PhysXBoxCollider box({1.f, 1.f, 1.f}, {10.f, 0.f, 0.f});
    const auto p = box.find_abs_furthest_vertex_position({1.f, 1.f, 1.f});
    EXPECT_FLOAT_EQ(p.x, 11.f);
    EXPECT_FLOAT_EQ(p.y,  1.f);
    EXPECT_FLOAT_EQ(p.z,  1.f);
}

TEST(PhysXBoxCollider, SetHalfExtentsUpdatesGeometry)
{
    PhysXBoxCollider box({1.f, 1.f, 1.f});
    box.set_half_extents({5.f, 6.f, 7.f});

    const auto& he = box.get_geometry().halfExtents;
    EXPECT_FLOAT_EQ(he.x, 5.f);
    EXPECT_FLOAT_EQ(he.y, 6.f);
    EXPECT_FLOAT_EQ(he.z, 7.f);

    // Furthest vertex must reflect the new extents.
    const auto p = box.find_abs_furthest_vertex_position({1.f, 1.f, 1.f});
    EXPECT_FLOAT_EQ(p.x, 5.f);
    EXPECT_FLOAT_EQ(p.y, 6.f);
    EXPECT_FLOAT_EQ(p.z, 7.f);
}

// ─── PhysXSphereCollider ─────────────────────────────────────────────────────

TEST(PhysXSphereCollider, DefaultOriginIsZero)
{
    PhysXSphereCollider sphere(1.f);
    EXPECT_EQ(sphere.get_origin(), Vector3<float>(0.f, 0.f, 0.f));
}

TEST(PhysXSphereCollider, SetAndGetOrigin)
{
    PhysXSphereCollider sphere(1.f, {1.f, 2.f, 3.f});
    EXPECT_EQ(sphere.get_origin(), Vector3<float>(1.f, 2.f, 3.f));

    sphere.set_origin({-5.f, 0.f, 0.f});
    EXPECT_EQ(sphere.get_origin(), Vector3<float>(-5.f, 0.f, 0.f));
}

TEST(PhysXSphereCollider, FurthestPointAlongPureXAxis)
{
    // Direction (1,0,0), radius 3 → furthest point is (3, 0, 0).
    PhysXSphereCollider sphere(3.f);
    const auto p = sphere.find_abs_furthest_vertex_position({1.f, 0.f, 0.f});
    EXPECT_FLOAT_EQ(p.x, 3.f);
    EXPECT_FLOAT_EQ(p.y, 0.f);
    EXPECT_FLOAT_EQ(p.z, 0.f);
}

TEST(PhysXSphereCollider, FurthestPointAlongDiagonal)
{
    // Direction (1,1,0), radius 1 → furthest point at distance 1 from origin.
    PhysXSphereCollider sphere(1.f);
    const auto p = sphere.find_abs_furthest_vertex_position({1.f, 1.f, 0.f});
    const float dist = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
    EXPECT_NEAR(dist, 1.f, 1e-5f);
}

TEST(PhysXSphereCollider, FurthestPointWithNonZeroOrigin)
{
    // Sphere at (5, 0, 0), radius 2. Direction +x → (7, 0, 0).
    PhysXSphereCollider sphere(2.f, {5.f, 0.f, 0.f});
    const auto p = sphere.find_abs_furthest_vertex_position({1.f, 0.f, 0.f});
    EXPECT_FLOAT_EQ(p.x, 7.f);
    EXPECT_FLOAT_EQ(p.y, 0.f);
    EXPECT_FLOAT_EQ(p.z, 0.f);
}

TEST(PhysXSphereCollider, ZeroDirectionReturnsOrigin)
{
    PhysXSphereCollider sphere(5.f, {1.f, 2.f, 3.f});
    const auto p = sphere.find_abs_furthest_vertex_position({0.f, 0.f, 0.f});
    EXPECT_EQ(p, sphere.get_origin());
}

TEST(PhysXSphereCollider, SetRadiusUpdatesGeometry)
{
    PhysXSphereCollider sphere(1.f);
    sphere.set_radius(10.f);
    EXPECT_FLOAT_EQ(sphere.get_radius(), 10.f);

    // Furthest point along +x should now be at x = 10.
    const auto p = sphere.find_abs_furthest_vertex_position({1.f, 0.f, 0.f});
    EXPECT_FLOAT_EQ(p.x, 10.f);
}

// ─── GJK: Box vs Box ─────────────────────────────────────────────────────────

using GjkBox    = omath::collision::GjkAlgorithm<PhysXBoxCollider>;
using GjkSphere = omath::collision::GjkAlgorithm<PhysXSphereCollider>;

TEST(PhysXBoxGjk, CollidingOverlap)
{
    // Two unit boxes: A at origin, B shifted by 0.5 — clearly overlapping.
    const PhysXBoxCollider a({1.f, 1.f, 1.f});
    const PhysXBoxCollider b({1.f, 1.f, 1.f}, {0.5f, 0.f, 0.f});
    EXPECT_TRUE(GjkBox::is_collide(a, b));
}

TEST(PhysXBoxGjk, CollidingTouching)
{
    // Boxes exactly touching on the +X face: A[-1,1] and B[1,3] along X.
    const PhysXBoxCollider a({1.f, 1.f, 1.f});
    const PhysXBoxCollider b({1.f, 1.f, 1.f}, {2.f, 0.f, 0.f});
    EXPECT_TRUE(GjkBox::is_collide(a, b));
}

TEST(PhysXBoxGjk, NotCollidingSeparated)
{
    // Boxes separated by a gap: A[-1,1] and B[3,5] along X.
    const PhysXBoxCollider a({1.f, 1.f, 1.f});
    const PhysXBoxCollider b({1.f, 1.f, 1.f}, {4.f, 0.f, 0.f});
    EXPECT_FALSE(GjkBox::is_collide(a, b));
}

TEST(PhysXBoxGjk, CollidingSameOrigin)
{
    // Same position — fully overlapping.
    const PhysXBoxCollider a({1.f, 1.f, 1.f});
    const PhysXBoxCollider b({1.f, 1.f, 1.f});
    EXPECT_TRUE(GjkBox::is_collide(a, b));
}

TEST(PhysXBoxGjk, NotCollidingDiagonalSeparation)
{
    // Boxes separated along a diagonal so no axis-aligned faces overlap.
    const PhysXBoxCollider a({1.f, 1.f, 1.f});
    const PhysXBoxCollider b({1.f, 1.f, 1.f}, {3.f, 3.f, 3.f});
    EXPECT_FALSE(GjkBox::is_collide(a, b));
}

TEST(PhysXBoxGjk, DifferentSizesColliding)
{
    // Large box vs small box inside it.
    const PhysXBoxCollider large({5.f, 5.f, 5.f});
    const PhysXBoxCollider small_box({1.f, 1.f, 1.f}, {2.f, 0.f, 0.f});
    EXPECT_TRUE(GjkBox::is_collide(large, small_box));
}

// ─── GJK: Sphere vs Sphere ───────────────────────────────────────────────────

TEST(PhysXSphereGjk, CollidingOverlap)
{
    // Radii 1 each, centres 1 apart — overlapping.
    const PhysXSphereCollider a(1.f);
    const PhysXSphereCollider b(1.f, {1.f, 0.f, 0.f});
    EXPECT_TRUE(GjkSphere::is_collide(a, b));
}

TEST(PhysXSphereGjk, CollidingSameOrigin)
{
    const PhysXSphereCollider a(1.f);
    const PhysXSphereCollider b(1.f);
    EXPECT_TRUE(GjkSphere::is_collide(a, b));
}

TEST(PhysXSphereGjk, NotCollidingSeparated)
{
    // Radii 1 each, centres 3 apart — gap of 1.
    const PhysXSphereCollider a(1.f);
    const PhysXSphereCollider b(1.f, {3.f, 0.f, 0.f});
    EXPECT_FALSE(GjkSphere::is_collide(a, b));
}

TEST(PhysXSphereGjk, DifferentRadiiColliding)
{
    // r=2 and r=1, centres 2.5 apart — still overlapping.
    const PhysXSphereCollider a(2.f);
    const PhysXSphereCollider b(1.f, {2.5f, 0.f, 0.f});
    EXPECT_TRUE(GjkSphere::is_collide(a, b));
}

TEST(PhysXSphereGjk, DifferentRadiiNotColliding)
{
    // r=1 and r=1, centres 5 apart — separated.
    const PhysXSphereCollider a(1.f);
    const PhysXSphereCollider b(1.f, {5.f, 0.f, 0.f});
    EXPECT_FALSE(GjkSphere::is_collide(a, b));
}

#endif // OMATH_ENABLE_PHYSX
