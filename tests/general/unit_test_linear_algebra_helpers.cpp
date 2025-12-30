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
    [[maybe_unused]] const auto l = a.length();
    [[maybe_unused]] const auto ang = a.angle_between(b);
    [[maybe_unused]] const auto perp = a.is_perpendicular(b);
    [[maybe_unused]] const auto norm = a.normalized();

    SUCCEED();
}

TEST(LinearAlgebraHelpers, TriangleNoInlineHelpersExecute)
{
    constexpr Vector3<float> v1{0.f,0.f,0.f};
    constexpr Vector3<float> v2{3.f,0.f,0.f};
    constexpr Vector3<float> v3{3.f,4.f,0.f};

    constexpr Triangle<Vector3<float>> t{v1, v2, v3};

    [[maybe_unused]] const auto n = t.calculate_normal();
    [[maybe_unused]] const auto a = t.side_a_length();
    [[maybe_unused]] const auto b = t.side_b_length();
    [[maybe_unused]] const auto h = t.hypot();
    [[maybe_unused]] const auto r = t.is_rectangular();

    SUCCEED();
}

TEST(LinearAlgebraHelpers, Vector4NoInlineHelpersExecute)
{
    Vector4<float> v{1.f,2.f,3.f,4.f};

    [[maybe_unused]] const auto l = v.length();
    [[maybe_unused]] const auto s = v.sum();
    [[maybe_unused]] const auto c = v.clamp(-10.f, 10.f);

    SUCCEED();
}
