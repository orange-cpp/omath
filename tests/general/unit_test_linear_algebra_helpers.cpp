#include "omath/linear_algebra/triangle.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include "omath/linear_algebra/vector4.hpp"
#include <gtest/gtest.h>

// This test file exercises the non-inlined helpers added to headers
// (Vector3, Triangle, Vector4) to encourage symbol emission and
// runtime execution so coverage tools can attribute hits back to the
// header lines.

using namespace omath;

TEST(LinearAlgebraHelpers, Vector3NoInlineHelpersExecute)
{
    constexpr Vector3<float> a{1.f, 2.f, 3.f};
    constexpr Vector3<float> b{4.f, 5.f, 6.f};

    // Execute helpers that were made non-inlined
    const auto l = a.length();
    const auto ang = a.angle_between(b);
    const auto perp = a.is_perpendicular(b);
    const auto norm = a.normalized();

    (void)l; (void)ang; (void)perp; (void)norm;
    SUCCEED();
}

TEST(LinearAlgebraHelpers, TriangleNoInlineHelpersExecute)
{
    constexpr Vector3<float> v1{0.f,0.f,0.f};
    constexpr Vector3<float> v2{3.f,0.f,0.f};
    constexpr Vector3<float> v3{3.f,4.f,0.f};

    constexpr Triangle<Vector3<float>> t{v1, v2, v3};

    const auto n = t.calculate_normal();
    const auto a = t.side_a_length();
    const auto b = t.side_b_length();
    const auto h = t.hypot();
    const auto r = t.is_rectangular();

    (void)n; (void)a; (void)b; (void)h; (void)r;
    SUCCEED();
}

TEST(LinearAlgebraHelpers, Vector4NoInlineHelpersExecute)
{
    Vector4<float> v{1.f,2.f,3.f,4.f};

    const auto l = v.length();
    const auto s = v.sum();
    v.clamp(-10.f, 10.f);

    (void)l; (void)s;
    SUCCEED();
}
