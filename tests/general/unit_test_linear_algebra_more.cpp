#include "omath/linear_algebra/triangle.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include "omath/linear_algebra/vector4.hpp"
#include <gtest/gtest.h>

using namespace omath;

TEST(LinearAlgebraMore, Vector3EdgeCases)
{
    constexpr Vector3<float> zero{0.f,0.f,0.f};
    constexpr Vector3<float> v{1.f,0.f,0.f};

    // angle_between should be unexpected when one vector has zero length
    const auto angle = zero.angle_between(v);
    EXPECT_FALSE(static_cast<bool>(angle));

    // normalized of zero should return zero
    const auto nz = zero.normalized();
    EXPECT_EQ(nz.x, 0.f);
    EXPECT_EQ(nz.y, 0.f);
    EXPECT_EQ(nz.z, 0.f);

    // perpendicular case: x-axis and y-axis
    constexpr Vector3<float> x{1.f,0.f,0.f};
    constexpr Vector3<float> y{0.f,1.f,0.f};
    EXPECT_TRUE(x.is_perpendicular(y));
}

TEST(LinearAlgebraMore, TriangleRectangularAndDegenerate)
{
    constexpr Vector3<float> v1{0.f,0.f,0.f};
    constexpr Vector3<float> v2{3.f,0.f,0.f};
    constexpr Vector3<float> v3{3.f,4.f,0.f}; // 3-4-5 triangle, rectangular at v2

    constexpr Triangle<Vector3<float>> t{v1,v2,v3};

    EXPECT_NEAR(t.side_a_length(), 3.f, 1e-6f);
    EXPECT_NEAR(t.side_b_length(), 4.f, 1e-6f);
    EXPECT_NEAR(t.hypot(), 5.f, 1e-6f);
    EXPECT_TRUE(t.is_rectangular());

    // Degenerate: all points same
    const Triangle<Vector3<float>> d{v1,v1,v1};
    EXPECT_NEAR(d.side_a_length(), 0.f, 1e-6f);
    EXPECT_NEAR(d.side_b_length(), 0.f, 1e-6f);
    EXPECT_NEAR(d.hypot(), 0.f, 1e-6f);
}

TEST(LinearAlgebraMore, Vector4ClampAndComparisons)
{
    Vector4<float> v{10.f, -20.f, 30.f, -40.f};
    const auto s = v.sum();
    EXPECT_NEAR(s, -20.f, 1e-6f);

    v.clamp(-10.f, 10.f);
    EXPECT_LE(v.x, 10.f);
    EXPECT_GE(v.x, -10.f);
    EXPECT_LE(v.y, 10.f);
    EXPECT_GE(v.y, -10.f);

    constexpr Vector4<float> a{1.f,2.f,3.f,4.f};
    constexpr Vector4<float> b{2.f,2.f,2.f,2.f};
    EXPECT_TRUE(a < b || a > b || a == b); // just exercise comparisons
}
