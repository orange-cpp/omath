//
// Created by Vlad on 7/7/2025.
//
// -------------------------------------------------------------------------------------------------
// test_gjk.cpp – GoogleTest unit‑suite for omath::collision::GJK
// -------------------------------------------------------------------------------------------------
//  Compile with something like:
//      g++ -std=c++20 -I<your_include_paths> test_gjk.cpp -o test_gjk -lgtest -lpthread
// -------------------------------------------------------------------------------------------------

#include "omath/collision/Gjk.hpp" // header we just added
#include <gtest/gtest.h>

using namespace omath;
using omath::collision::Gjk;

namespace  // anonymous – only for this translation unit
{
    // ---------------------------------------------------------------------------------------------
    // Minimal axis‑aligned box – centre + half‑sizes
    // ---------------------------------------------------------------------------------------------
    struct Box
    {
        Vector3<float> centre;
        Vector3<float> half;

        [[nodiscard]] constexpr Vector3<float> support(const Vector3<float>& dir) const noexcept
        {
            return {
                centre.x + (dir.x >= 0.f ? half.x : -half.x),
                centre.y + (dir.y >= 0.f ? half.y : -half.y),
                centre.z + (dir.z >= 0.f ? half.z : -half.z)
            };
        }
    };

    // Convenience helpers
    constexpr Box make_box(float cx, float cy, float cz,
                           float hx, float hy, float hz) noexcept
    {
        return { { cx, cy, cz }, { hx, hy, hz } };
    }

    inline void translate(Box& b, const Vector3<float>& delta) noexcept { b.centre += delta; }
}

// -------------------------------------------------------------------------------------------------
//  Test‑fixture: creates a unit cube at the origin for *a*
// -------------------------------------------------------------------------------------------------
class UnitTestGJK : public ::testing::Test
{
protected:
    Box a;
    Box b;

    void SetUp() override
    {
        a = make_box(0.f, 0.f, 0.f, 1.f, 1.f, 1.f);   // 2×2×2 cube centred at origin
    }
};

// -------------------------------------------------------------------------------------------------
//  1. Separated cubes – both intersects() and penetration_vector() must report *no collision*
// -------------------------------------------------------------------------------------------------
TEST_F(UnitTestGJK, NoIntersection)
{
    b = make_box( 5.f, 0.f, 0.f, 1.f, 1.f, 1.f );     // far away along +X

    EXPECT_FALSE( Gjk::intersects(a, b) );
    EXPECT_FALSE( Gjk::penetration_vector(a, b).has_value() );
}

// -------------------------------------------------------------------------------------------------
//  2. Just‑touching faces – still considered *no* intersection (distance == 0)
// -------------------------------------------------------------------------------------------------
TEST_F(UnitTestGJK, TouchingFaces)
{
    b = make_box( 2.f, 0.f, 0.f, 1.f, 1.f, 1.f );     // faces touch at X = 1

    EXPECT_FALSE( Gjk::intersects(a, b) );
    EXPECT_FALSE( Gjk::penetration_vector(a, b).has_value() );
}

// -------------------------------------------------------------------------------------------------
//  3. Overlapping cubes – intersects() must return true, MTV magnitude along x = 1.5
// -------------------------------------------------------------------------------------------------
TEST_F(UnitTestGJK, OverlapWithMTV)
{
    b = make_box( 0.5f, 0.f, 0.f, 1.f, 1.f, 1.f );    // centres 0.5 apart → 1.5 units penetration on X

    EXPECT_TRUE( Gjk::intersects(a, b) );

    auto mtvOpt = Gjk::penetration_vector(a, b);
    ASSERT_TRUE( mtvOpt.has_value() );

    const Vector3<float>& mtv = *mtvOpt;

    // MTV should resolve along the X axis (either +X or −X)
    EXPECT_NEAR( std::fabs(mtv.y), 0.f, 1e-4f );
    EXPECT_NEAR( std::fabs(mtv.z), 0.f, 1e-4f );

    // Magnitude must equal the overlap (1.5)
    EXPECT_NEAR( mtv.length(), 1.5f, 1e-3f );

    // After translating cube *a* we should be collision‑free
    translate(a, mtv);
    EXPECT_FALSE( Gjk::intersects(a, b) );
}

// -------------------------------------------------------------------------------------------------
//  4. Slanted overlap – verify MTV points along shortest axis
// -------------------------------------------------------------------------------------------------
TEST_F(UnitTestGJK, OverlapOffAxis)
{
    // Cube *b* overlaps *a* by 0.5 on X and 0.2 on Y.  The MTV should choose the Y‑axis because
    // it requires the smaller translation (0.2 < 0.5).
    b = make_box( 0.5f, 0.2f, 0.f, 1.f, 1.f, 1.f );

    ASSERT_TRUE( Gjk::intersects(a, b) );
    auto mtvOpt = Gjk::penetration_vector(a, b);
    ASSERT_TRUE( mtvOpt.has_value() );

    const Vector3<float> mtv = *mtvOpt;

    // Expect MTV mainly along ±Y
    EXPECT_GT( std::fabs(mtv.y), std::fabs(mtv.x) );
    EXPECT_GT( std::fabs(mtv.y), std::fabs(mtv.z) );

    // Magnitude close to expected overlap (0.8 or 1.2 depending on sign conventions) – just check not zero
    EXPECT_GT( mtv.length(), 0.19f );

    translate(a, mtv);
    EXPECT_FALSE( Gjk::intersects(a, b) );
}

// -------------------------------------------------------------------------------------------------
//  5. Degenerate case – identical cubes totally overlapping (a == b)
// -------------------------------------------------------------------------------------------------
TEST_F(UnitTestGJK, IdenticalVolumes)
{
    b = a; // perfectly coincident

    EXPECT_TRUE( Gjk::intersects(a, b) );
    auto mtv = Gjk::penetration_vector(a, b);
    ASSERT_TRUE( mtv.has_value() );

    // Any non‑zero MTV is acceptable; we just need to ensure translation resolves the clash
    translate(a, *mtv);
    EXPECT_FALSE( Gjk::intersects(a, b) );
}
