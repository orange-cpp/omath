//
// Created by Vladislav on 19.04.2026.
//
#include <gtest/gtest.h>
#include "omath/3d_primitives/aabb.hpp"

using AABB = omath::primitives::Aabb<float>;
using Vec3 = omath::Vector3<float>;

// --- center() ---

TEST(AabbTests, CenterOfSymmetricBox)
{
    const AABB box{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    const auto c = box.center();
    EXPECT_FLOAT_EQ(c.x, 0.f);
    EXPECT_FLOAT_EQ(c.y, 0.f);
    EXPECT_FLOAT_EQ(c.z, 0.f);
}

TEST(AabbTests, CenterOfOffsetBox)
{
    const AABB box{{1.f, 2.f, 3.f}, {3.f, 6.f, 7.f}};
    const auto c = box.center();
    EXPECT_FLOAT_EQ(c.x, 2.f);
    EXPECT_FLOAT_EQ(c.y, 4.f);
    EXPECT_FLOAT_EQ(c.z, 5.f);
}

TEST(AabbTests, CenterOfDegenerateBox)
{
    const AABB box{{5.f, 5.f, 5.f}, {5.f, 5.f, 5.f}};
    const auto c = box.center();
    EXPECT_FLOAT_EQ(c.x, 5.f);
    EXPECT_FLOAT_EQ(c.y, 5.f);
    EXPECT_FLOAT_EQ(c.z, 5.f);
}

// --- extents() ---

TEST(AabbTests, ExtentsOfSymmetricBox)
{
    const AABB box{{-2.f, -3.f, -4.f}, {2.f, 3.f, 4.f}};
    const auto e = box.extents();
    EXPECT_FLOAT_EQ(e.x, 2.f);
    EXPECT_FLOAT_EQ(e.y, 3.f);
    EXPECT_FLOAT_EQ(e.z, 4.f);
}

TEST(AabbTests, ExtentsOfUnitBox)
{
    const AABB box{{0.f, 0.f, 0.f}, {2.f, 2.f, 2.f}};
    const auto e = box.extents();
    EXPECT_FLOAT_EQ(e.x, 1.f);
    EXPECT_FLOAT_EQ(e.y, 1.f);
    EXPECT_FLOAT_EQ(e.z, 1.f);
}

TEST(AabbTests, ExtentsOfDegenerateBox)
{
    const AABB box{{3.f, 3.f, 3.f}, {3.f, 3.f, 3.f}};
    const auto e = box.extents();
    EXPECT_FLOAT_EQ(e.x, 0.f);
    EXPECT_FLOAT_EQ(e.y, 0.f);
    EXPECT_FLOAT_EQ(e.z, 0.f);
}

// --- is_collide() ---

TEST(AabbTests, OverlappingBoxesCollide)
{
    const AABB a{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    const AABB b{{0.f, 0.f, 0.f}, {2.f, 2.f, 2.f}};
    EXPECT_TRUE(a.is_collide(b));
    EXPECT_TRUE(b.is_collide(a));
}

TEST(AabbTests, SeparatedBoxesDoNotCollide)
{
    const AABB a{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    const AABB b{{2.f, 2.f, 2.f}, {4.f, 4.f, 4.f}};
    EXPECT_FALSE(a.is_collide(b));
    EXPECT_FALSE(b.is_collide(a));
}

TEST(AabbTests, TouchingFacesCollide)
{
    const AABB a{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    const AABB b{{1.f, -1.f, -1.f}, {3.f, 1.f, 1.f}};
    EXPECT_TRUE(a.is_collide(b));
    EXPECT_TRUE(b.is_collide(a));
}

TEST(AabbTests, ContainedBoxCollides)
{
    const AABB outer{{-3.f, -3.f, -3.f}, {3.f, 3.f, 3.f}};
    const AABB inner{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    EXPECT_TRUE(outer.is_collide(inner));
    EXPECT_TRUE(inner.is_collide(outer));
}

TEST(AabbTests, SeparatedOnXAxisDoNotCollide)
{
    const AABB a{{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}};
    const AABB b{{2.f, 0.f, 0.f}, {3.f, 1.f, 1.f}};
    EXPECT_FALSE(a.is_collide(b));
}

TEST(AabbTests, SeparatedOnYAxisDoNotCollide)
{
    const AABB a{{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}};
    const AABB b{{0.f, 2.f, 0.f}, {1.f, 3.f, 1.f}};
    EXPECT_FALSE(a.is_collide(b));
}

TEST(AabbTests, SeparatedOnZAxisDoNotCollide)
{
    const AABB a{{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}};
    const AABB b{{0.f, 0.f, 2.f}, {1.f, 1.f, 3.f}};
    EXPECT_FALSE(a.is_collide(b));
}

TEST(AabbTests, IdenticalBoxesCollide)
{
    const AABB a{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    EXPECT_TRUE(a.is_collide(a));
}

TEST(AabbTests, DegeneratePointBoxCollidesWhenInsideOther)
{
    const AABB box{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    const AABB point{{0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}};
    EXPECT_TRUE(box.is_collide(point));
    EXPECT_TRUE(point.is_collide(box));
}

TEST(AabbTests, DegeneratePointBoxDoesNotCollideWhenOutside)
{
    const AABB box{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    const AABB point{{5.f, 0.f, 0.f}, {5.f, 0.f, 0.f}};
    EXPECT_FALSE(box.is_collide(point));
    EXPECT_FALSE(point.is_collide(box));
}
