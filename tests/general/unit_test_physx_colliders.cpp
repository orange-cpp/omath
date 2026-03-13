//
// Created by orange-cpp
//
#ifdef OMATH_ENABLE_PHYSX

#include <gtest/gtest.h>
#include <omath/collision/gjk_algorithm.hpp>
#include <omath/collision/physx_box_collider.hpp>
#include <omath/collision/physx_rigid_body.hpp>
#include <omath/collision/physx_sphere_collider.hpp>
#include <omath/collision/physx_world.hpp>

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

TEST(PhysXBoxGjk, NotCollidingTouching)
{
    // Boxes exactly touching on the +X face: A[-1,1] and B[1,3] along X.
    // GJK treats boundary contact (Minkowski difference passes through origin) as non-collision.
    const PhysXBoxCollider a({1.f, 1.f, 1.f});
    const PhysXBoxCollider b({1.f, 1.f, 1.f}, {2.f, 0.f, 0.f});
    EXPECT_FALSE(GjkBox::is_collide(a, b));
}

TEST(PhysXBoxGjk, CollidingSlightOverlap)
{
    // Boxes overlapping by 0.1 along X: A[-1,1] and B[0.9,2.9].
    const PhysXBoxCollider a({1.f, 1.f, 1.f});
    const PhysXBoxCollider b({1.f, 1.f, 1.f}, {1.9f, 0.f, 0.f});
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

// ─── PhysX simulation-based collision resolution ─────────────────────────────

// Helper: step the world N times with a fixed dt.
static void step_n(omath::collision::PhysXWorld& world, int n, float dt = 1.f / 60.f)
{
    for (int i = 0; i < n; ++i)
        world.step(dt);
}

TEST(PhysXSimulation, BoxFallsAndStopsOnGround)
{
    // A box dropped from y=5 should come to rest at y≈0.5 (half-extent) above the ground plane.
    omath::collision::PhysXWorld world;
    world.add_ground_plane(0.f);

    omath::collision::PhysXRigidBody box(world, physx::PxBoxGeometry(0.5f, 0.5f, 0.5f),
                                         {0.f, 5.f, 0.f});

    step_n(world, 300); // ~5 simulated seconds

    EXPECT_NEAR(box.get_origin().y, 0.5f, 0.05f);
}

TEST(PhysXSimulation, SphereFallsAndStopsOnGround)
{
    // A sphere of radius 1 dropped from y=5 should rest at y≈1.
    omath::collision::PhysXWorld world;
    world.add_ground_plane(0.f);

    omath::collision::PhysXRigidBody sphere(world, physx::PxSphereGeometry(1.f),
                                            {0.f, 5.f, 0.f});

    step_n(world, 300);

    EXPECT_NEAR(sphere.get_origin().y, 1.f, 0.05f);
}

TEST(PhysXSimulation, TwoBoxesCollideSeparate)
{
    // Two boxes launched toward each other — after collision they must be
    // further apart than their combined half-extents (no overlap).
    omath::collision::PhysXWorld world({0.f, 0.f, 0.f}); // no gravity

    omath::collision::PhysXRigidBody left (world, physx::PxBoxGeometry(0.5f, 0.5f, 0.5f), {-3.f, 0.f, 0.f});
    omath::collision::PhysXRigidBody right(world, physx::PxBoxGeometry(0.5f, 0.5f, 0.5f), { 3.f, 0.f, 0.f});

    left.set_linear_velocity({ 5.f, 0.f, 0.f});
    right.set_linear_velocity({-5.f, 0.f, 0.f});

    step_n(world, 120); // 2 simulated seconds

    const float distance = right.get_origin().x - left.get_origin().x;
    // Boxes must not be overlapping (combined extents = 1.0).
    EXPECT_GE(distance, 1.0f);
}

TEST(PhysXSimulation, BoxGetOriginMatchesSetOrigin)
{
    // Kinematic teleport — set_origin must immediately reflect in get_origin.
    omath::collision::PhysXWorld world;
    omath::collision::PhysXRigidBody box(world, physx::PxBoxGeometry(1.f, 1.f, 1.f));
    box.set_kinematic(true);

    box.set_origin({7.f, 3.f, -2.f});

    EXPECT_NEAR(box.get_origin().x,  7.f, 1e-4f);
    EXPECT_NEAR(box.get_origin().y,  3.f, 1e-4f);
    EXPECT_NEAR(box.get_origin().z, -2.f, 1e-4f);
}

TEST(PhysXSimulation, BoxFallsUnderGravity)
{
    // Without a floor, a box should be lower after simulation than its start.
    omath::collision::PhysXWorld world; // default gravity -9.81 Y
    omath::collision::PhysXRigidBody box(world, physx::PxBoxGeometry(0.5f, 0.5f, 0.5f),
                                         {0.f, 10.f, 0.f});

    const float y_start = box.get_origin().y;
    step_n(world, 60); // 1 simulated second

    EXPECT_LT(box.get_origin().y, y_start);
}

#endif // OMATH_ENABLE_PHYSX
