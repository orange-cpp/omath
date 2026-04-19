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
    constexpr AABB box{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    constexpr auto c = box.center();
    EXPECT_FLOAT_EQ(c.x, 0.f);
    EXPECT_FLOAT_EQ(c.y, 0.f);
    EXPECT_FLOAT_EQ(c.z, 0.f);
}

TEST(AabbTests, CenterOfOffsetBox)
{
    constexpr AABB box{{1.f, 2.f, 3.f}, {3.f, 6.f, 7.f}};
    constexpr auto c = box.center();
    EXPECT_FLOAT_EQ(c.x, 2.f);
    EXPECT_FLOAT_EQ(c.y, 4.f);
    EXPECT_FLOAT_EQ(c.z, 5.f);
}

TEST(AabbTests, CenterOfDegenerateBox)
{
    constexpr AABB box{{5.f, 5.f, 5.f}, {5.f, 5.f, 5.f}};
    constexpr auto c = box.center();
    EXPECT_FLOAT_EQ(c.x, 5.f);
    EXPECT_FLOAT_EQ(c.y, 5.f);
    EXPECT_FLOAT_EQ(c.z, 5.f);
}

// --- extents() ---

TEST(AabbTests, ExtentsOfSymmetricBox)
{
    constexpr AABB box{{-2.f, -3.f, -4.f}, {2.f, 3.f, 4.f}};
    constexpr auto e = box.extents();
    EXPECT_FLOAT_EQ(e.x, 2.f);
    EXPECT_FLOAT_EQ(e.y, 3.f);
    EXPECT_FLOAT_EQ(e.z, 4.f);
}

TEST(AabbTests, ExtentsOfUnitBox)
{
    constexpr AABB box{{0.f, 0.f, 0.f}, {2.f, 2.f, 2.f}};
    constexpr auto e = box.extents();
    EXPECT_FLOAT_EQ(e.x, 1.f);
    EXPECT_FLOAT_EQ(e.y, 1.f);
    EXPECT_FLOAT_EQ(e.z, 1.f);
}

TEST(AabbTests, ExtentsOfDegenerateBox)
{
    constexpr AABB box{{3.f, 3.f, 3.f}, {3.f, 3.f, 3.f}};
    constexpr auto e = box.extents();
    EXPECT_FLOAT_EQ(e.x, 0.f);
    EXPECT_FLOAT_EQ(e.y, 0.f);
    EXPECT_FLOAT_EQ(e.z, 0.f);
}

using UpAxis = omath::primitives::UpAxis;

// --- top() ---

TEST(AabbTests, TopYUpSymmetricBox)
{
    constexpr AABB box{{-1.f, -2.f, -3.f}, {1.f, 2.f, 3.f}};
    constexpr auto t = box.top<UpAxis::Y>();
    EXPECT_FLOAT_EQ(t.x, 0.f);
    EXPECT_FLOAT_EQ(t.y, 2.f);
    EXPECT_FLOAT_EQ(t.z, 0.f);
}

TEST(AabbTests, TopYUpOffsetBox)
{
    constexpr AABB box{{1.f, 4.f, 2.f}, {3.f, 10.f, 6.f}};
    constexpr auto t = box.top<UpAxis::Y>();
    EXPECT_FLOAT_EQ(t.x, 2.f);
    EXPECT_FLOAT_EQ(t.y, 10.f);
    EXPECT_FLOAT_EQ(t.z, 4.f);
}

TEST(AabbTests, TopZUpSymmetricBox)
{
    constexpr AABB box{{-1.f, -2.f, -3.f}, {1.f, 2.f, 3.f}};
    constexpr auto t = box.top<UpAxis::Z>();
    EXPECT_FLOAT_EQ(t.x, 0.f);
    EXPECT_FLOAT_EQ(t.y, 0.f);
    EXPECT_FLOAT_EQ(t.z, 3.f);
}

TEST(AabbTests, TopZUpOffsetBox)
{
    constexpr AABB box{{1.f, 4.f, 2.f}, {3.f, 10.f, 6.f}};
    constexpr auto t = box.top<UpAxis::Z>();
    EXPECT_FLOAT_EQ(t.x, 2.f);
    EXPECT_FLOAT_EQ(t.y, 7.f);
    EXPECT_FLOAT_EQ(t.z, 6.f);
}

TEST(AabbTests, TopDefaultIsYUp)
{
    constexpr AABB box{{0.f, 0.f, 0.f}, {2.f, 4.f, 6.f}};
    EXPECT_EQ(box.top(), box.top<UpAxis::Y>());
}

// --- bottom() ---

TEST(AabbTests, BottomYUpSymmetricBox)
{
    constexpr AABB box{{-1.f, -2.f, -3.f}, {1.f, 2.f, 3.f}};
    constexpr auto b = box.bottom<UpAxis::Y>();
    EXPECT_FLOAT_EQ(b.x, 0.f);
    EXPECT_FLOAT_EQ(b.y, -2.f);
    EXPECT_FLOAT_EQ(b.z, 0.f);
}

TEST(AabbTests, BottomYUpOffsetBox)
{
    constexpr AABB box{{1.f, 4.f, 2.f}, {3.f, 10.f, 6.f}};
    constexpr auto b = box.bottom<UpAxis::Y>();
    EXPECT_FLOAT_EQ(b.x, 2.f);
    EXPECT_FLOAT_EQ(b.y, 4.f);
    EXPECT_FLOAT_EQ(b.z, 4.f);
}

TEST(AabbTests, BottomZUpSymmetricBox)
{
    constexpr AABB box{{-1.f, -2.f, -3.f}, {1.f, 2.f, 3.f}};
    constexpr auto b = box.bottom<UpAxis::Z>();
    EXPECT_FLOAT_EQ(b.x, 0.f);
    EXPECT_FLOAT_EQ(b.y, 0.f);
    EXPECT_FLOAT_EQ(b.z, -3.f);
}

TEST(AabbTests, BottomZUpOffsetBox)
{
    constexpr AABB box{{1.f, 4.f, 2.f}, {3.f, 10.f, 6.f}};
    constexpr auto b = box.bottom<UpAxis::Z>();
    EXPECT_FLOAT_EQ(b.x, 2.f);
    EXPECT_FLOAT_EQ(b.y, 7.f);
    EXPECT_FLOAT_EQ(b.z, 2.f);
}

TEST(AabbTests, BottomDefaultIsYUp)
{
    constexpr AABB box{{0.f, 0.f, 0.f}, {2.f, 4.f, 6.f}};
    EXPECT_EQ(box.bottom(), box.bottom<UpAxis::Y>());
}

TEST(AabbTests, TopAndBottomAreSymmetric)
{
    constexpr AABB box{{-1.f, -2.f, -3.f}, {1.f, 2.f, 3.f}};
    EXPECT_FLOAT_EQ(box.top<UpAxis::Y>().y, -box.bottom<UpAxis::Y>().y);
    EXPECT_FLOAT_EQ(box.top<UpAxis::Z>().z, -box.bottom<UpAxis::Z>().z);
}

// --- is_collide() ---

TEST(AabbTests, OverlappingBoxesCollide)
{
    constexpr AABB a{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    constexpr AABB b{{0.f, 0.f, 0.f}, {2.f, 2.f, 2.f}};
    EXPECT_TRUE(a.is_collide(b));
    EXPECT_TRUE(b.is_collide(a));
}

TEST(AabbTests, SeparatedBoxesDoNotCollide)
{
    constexpr AABB a{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    constexpr AABB b{{2.f, 2.f, 2.f}, {4.f, 4.f, 4.f}};
    EXPECT_FALSE(a.is_collide(b));
    EXPECT_FALSE(b.is_collide(a));
}

TEST(AabbTests, TouchingFacesCollide)
{
    constexpr AABB a{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    constexpr AABB b{{1.f, -1.f, -1.f}, {3.f, 1.f, 1.f}};
    EXPECT_TRUE(a.is_collide(b));
    EXPECT_TRUE(b.is_collide(a));
}

TEST(AabbTests, ContainedBoxCollides)
{
    constexpr AABB outer{{-3.f, -3.f, -3.f}, {3.f, 3.f, 3.f}};
    constexpr AABB inner{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    EXPECT_TRUE(outer.is_collide(inner));
    EXPECT_TRUE(inner.is_collide(outer));
}

TEST(AabbTests, SeparatedOnXAxisDoNotCollide)
{
    constexpr AABB a{{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}};
    constexpr AABB b{{2.f, 0.f, 0.f}, {3.f, 1.f, 1.f}};
    EXPECT_FALSE(a.is_collide(b));
}

TEST(AabbTests, SeparatedOnYAxisDoNotCollide)
{
    constexpr AABB a{{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}};
    constexpr AABB b{{0.f, 2.f, 0.f}, {1.f, 3.f, 1.f}};
    EXPECT_FALSE(a.is_collide(b));
}

TEST(AabbTests, SeparatedOnZAxisDoNotCollide)
{
    constexpr AABB a{{0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}};
    constexpr AABB b{{0.f, 0.f, 2.f}, {1.f, 1.f, 3.f}};
    EXPECT_FALSE(a.is_collide(b));
}

TEST(AabbTests, IdenticalBoxesCollide)
{
    constexpr AABB a{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    EXPECT_TRUE(a.is_collide(a));
}

TEST(AabbTests, DegeneratePointBoxCollidesWhenInsideOther)
{
    constexpr AABB box{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    constexpr AABB point{{0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}};
    EXPECT_TRUE(box.is_collide(point));
    EXPECT_TRUE(point.is_collide(box));
}

TEST(AabbTests, DegeneratePointBoxDoesNotCollideWhenOutside)
{
    constexpr AABB box{{-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f}};
    constexpr AABB point{{5.f, 0.f, 0.f}, {5.f, 0.f, 0.f}};
    EXPECT_FALSE(box.is_collide(point));
    EXPECT_FALSE(point.is_collide(box));
}
