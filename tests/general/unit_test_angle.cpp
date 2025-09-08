//
// Created by Orange on 11/30/2024.
//

#include <cmath>
#include <gtest/gtest.h>
#include <numbers>
#include <omath/angle.hpp>

using namespace omath;

namespace
{

    // Handy aliases (defaults: Type=float, [0,360], Normalized)
    using Deg = Angle<float, float(0), float(360), AngleFlags::Normalized>;
    using Pitch = Angle<float, float(-90), float(90), AngleFlags::Clamped>;
    using Turn = Angle<float, float(-180), float(180), AngleFlags::Normalized>;

    constexpr float kEps = 1e-5f;

} // namespace

// ---------- Construction / factories ----------

TEST(UnitTestAngle, DefaultConstructor_IsZeroDegrees)
{
    Deg a; // default ctor
    EXPECT_FLOAT_EQ(*a, 0.0f);
    EXPECT_FLOAT_EQ(a.as_degrees(), 0.0f);
}

TEST(UnitTestAngle, FromDegrees_Normalized_WrapsAboveMax)
{
    const Deg a = Deg::from_degrees(370.0f);
    EXPECT_FLOAT_EQ(a.as_degrees(), 10.0f);
}

TEST(UnitTestAngle, FromDegrees_Normalized_WrapsBelowMin)
{
    const Deg a = Deg::from_degrees(-10.0f);
    EXPECT_FLOAT_EQ(a.as_degrees(), 350.0f);
}

TEST(UnitTestAngle, FromDegrees_Clamped_ClampsToRange)
{
    const Pitch hi = Pitch::from_degrees(100.0f);
    const Pitch lo = Pitch::from_degrees(-120.0f);

    EXPECT_FLOAT_EQ(hi.as_degrees(), 90.0f);
    EXPECT_FLOAT_EQ(lo.as_degrees(), -90.0f);
}

TEST(UnitTestAngle, FromRadians_And_AsRadians)
{
    const Deg a = Deg::from_radians(std::numbers::pi_v<float>);
    EXPECT_FLOAT_EQ(a.as_degrees(), 180.0f);

    const Deg b = Deg::from_degrees(180.0f);
    EXPECT_NEAR(b.as_radians(), std::numbers::pi_v<float>, 1e-6f);
}

// ---------- Unary minus & deref ----------

TEST(UnitTestAngle, UnaryMinus_Normalized)
{
    const Deg a = Deg::from_degrees(30.0f);
    const Deg b = -a; // wraps to 330 in [0,360)
    EXPECT_FLOAT_EQ(b.as_degrees(), 330.0f);
}

TEST(UnitTestAngle, DereferenceReturnsDegrees)
{
    const Deg a = Deg::from_degrees(42.0f);
    EXPECT_FLOAT_EQ(*a, 42.0f);
}

// ---------- Trigonometric helpers ----------

TEST(UnitTestAngle, SinCosTanCot_BasicCases)
{
    const Deg a0 = Deg::from_degrees(0.0f);
    EXPECT_NEAR(a0.sin(), 0.0f, kEps);
    EXPECT_NEAR(a0.cos(), 1.0f, kEps);
    // cot(0) -> cos/sin -> div by 0: allow inf or nan
    const float cot0 = a0.cot();
    EXPECT_TRUE(std::isinf(cot0) || std::isnan(cot0));

    const Deg a45 = Deg::from_degrees(45.0f);
    EXPECT_NEAR(a45.tan(), 1.0f, 1e-4f);
    EXPECT_NEAR(a45.cot(), 1.0f, 1e-4f);

    const Deg a90 = Deg::from_degrees(90.0f);
    EXPECT_NEAR(a90.sin(), 1.0f, 1e-4f);
    EXPECT_NEAR(a90.cos(), 0.0f, 1e-4f);
}

TEST(UnitTestAngle, Atan_IsAtanOfRadians)
{
    // atan(as_radians). For 0° -> atan(0)=0.
    const Deg a0 = Deg::from_degrees(0.0f);
    EXPECT_NEAR(a0.atan(), 0.0f, kEps);

    const Deg a45 = Deg::from_degrees(45.0f);
    // atan(pi/4) ≈ 0.665773...
    EXPECT_NEAR(a45.atan(), 0.66577375f, 1e-6f);
}

// ---------- Compound arithmetic ----------

TEST(UnitTestAngle, PlusEquals_Normalized_Wraps)
{
    Deg a = Deg::from_degrees(350.0f);
    a += Deg::from_degrees(20.0f); // 370 -> 10
    EXPECT_FLOAT_EQ(a.as_degrees(), 10.0f);
}

TEST(UnitTestAngle, MinusEquals_Normalized_Wraps)
{
    Deg a = Deg::from_degrees(10.0f);
    a -= Deg::from_degrees(30.0f); // -20 -> 340
    EXPECT_FLOAT_EQ(a.as_degrees(), 340.0f);
}

TEST(UnitTestAngle, PlusEquals_Clamped_Clamps)
{
    Pitch p = Pitch::from_degrees(80.0f);
    p += Pitch::from_degrees(30.0f); // 110 -> clamp to 90
    EXPECT_FLOAT_EQ(p.as_degrees(), 90.0f);
}

TEST(UnitTestAngle, MinusEquals_Clamped_Clamps)
{
    Pitch p = Pitch::from_degrees(-70.0f);
    p -= Pitch::from_degrees(40.0f); // -110 -> clamp to -90
    EXPECT_FLOAT_EQ(p.as_degrees(), -90.0f);
}

// ---------- Alternative ranges ----------

TEST(UnitTestAngle, NormalizedRange_Neg180To180)
{
    const Turn a = Turn::from_degrees(190.0f); // -> -170
    const Turn b = Turn::from_degrees(-190.0f); // -> 170

    EXPECT_FLOAT_EQ(a.as_degrees(), -170.0f);
    EXPECT_FLOAT_EQ(b.as_degrees(), 170.0f);
}

// ---------- Comparisons (via <=>) ----------

TEST(UnitTestAngle, Comparisons_WorkWithPartialOrdering)
{
    const Deg a = Deg::from_degrees(10.0f);
    const Deg b = Deg::from_degrees(20.0f);
    const Deg c = Deg::from_degrees(10.0f);

    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a <= c);
    EXPECT_TRUE(c >= a);
}

// ---------- std::format formatter ----------

TEST(UnitTestAngle, Formatter_PrintsDegreesWithSuffix)
{
    const Deg a = Deg::from_degrees(15.0f);
    EXPECT_EQ(std::format("{}", a), "15deg");

    const Deg b = Deg::from_degrees(10.5f);
    EXPECT_EQ(std::format("{}", b), "10.5deg");

    const Turn t = Turn::from_degrees(-170.0f);
    EXPECT_EQ(std::format("{}", t), "-170deg");
}

TEST(UnitTestAngle, BinaryPlus_ReturnsWrappedSum)
{
    Angle<> a = Deg::from_degrees(350.0f);
    const Deg b = Deg::from_degrees(20.0f);
    const Deg c = a + b; // expect 10°
    EXPECT_FLOAT_EQ(c.as_degrees(), 10.0f);
}

TEST(UnitTestAngle, BinaryMinus_ReturnsWrappedDiff)
{
    Angle<> a = Deg::from_degrees(10.0f);
    const Deg b = Deg::from_degrees(30.0f);
    const Deg c = a - b; // expect 340°
    EXPECT_FLOAT_EQ(c.as_degrees(), 340.0f);
}
