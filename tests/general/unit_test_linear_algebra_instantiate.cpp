// Instantiation-only tests to force out-of-line template emission
#include <gtest/gtest.h>
#include <format>
#include <functional>

#include "omath/linear_algebra/vector3.hpp"
#include "omath/linear_algebra/vector4.hpp"
#include "omath/linear_algebra/mat.hpp"

using namespace omath;

TEST(LinearAlgebraInstantiate, Vector3AndVector4AndMatCoverage) {
    // Vector3 usage
    Vector3<float> a{1.f, 2.f, 3.f};
    Vector3<float> b{4.f, 5.f, 6.f};

    // call various methods
    [[maybe_unused]] volatile float d0 = a.distance_to_sqr(b);
    [[maybe_unused]] volatile float d1 = a.dot(b);
    [[maybe_unused]] volatile auto c = a.cross(b);
    auto tup = a.as_tuple();
    [[maybe_unused]] volatile bool dir = a.point_to_same_direction(b);

    // non-inlined helpers
    [[maybe_unused]] volatile float ln = a.length();
    [[maybe_unused]] auto ang = a.angle_between(b);
    [[maybe_unused]] volatile bool perp = a.is_perpendicular(b, 0.1f);
    [[maybe_unused]] volatile auto anorm = a.normalized();

    // formatter and hash instantiations (char only)
    [[maybe_unused]] auto f1 = std::format("{}", a);
    [[maybe_unused]] auto h1 = std::hash<Vector3<float>>{}(a);

    // Vector4 usage
    Vector4<float> v4{1.f, -2.f, 3.f, -4.f};
    [[maybe_unused]] volatile float v4len = v4.length();
    [[maybe_unused]] volatile float v4sum = v4.sum();
    v4.clamp(-2.f, 2.f);
    [[maybe_unused]] auto f2 = std::format("{}", v4);
    [[maybe_unused]] auto h2 = std::hash<Vector4<float>>{}(v4);

    // Mat usage: instantiate several sizes and store orders
    [[maybe_unused]] Mat<1,1> m1{{42.f}};
    [[maybe_unused]] volatile float m1det = m1.determinant(); 

    Mat<2,2> m2{{{1.f,2.f},{3.f,4.f}}};
    float det2 = m2.determinant();
    [[maybe_unused]] auto tr2 = m2.transposed();
    [[maybe_unused]] auto minor00 = m2.minor(0,0);
    [[maybe_unused]] auto algc = m2.alg_complement(0,1);
    [[maybe_unused]] auto rarr = m2.raw_array();
    auto inv2 = m2.inverted();

    Mat<3,3> m3{{{1.f,2.f,3.f},{4.f,5.f,6.f},{7.f,8.f,9.f}}};
    [[maybe_unused]] volatile float det3 = m3.determinant();
    [[maybe_unused]] auto strip = m3.strip(0,0);
    [[maybe_unused]] auto min = m3.minor(2,2);

    // to_string/wstring/u8string and to_screen_mat
    [[maybe_unused]] auto s = m2.to_string();
    [[maybe_unused]] auto ws = m2.to_wstring();
    [[maybe_unused]] auto u8s = m2.to_u8string();
    [[maybe_unused]] auto screen = Mat<4,4>::to_screen_mat(800.f, 600.f);

    // call non-inlined mat helpers
    [[maybe_unused]] volatile auto det = m2.determinant();
    [[maybe_unused]] volatile auto inv = m2.inverted();
    [[maybe_unused]] volatile auto trans = m2.transposed();
    [[maybe_unused]] volatile auto raw = m2.raw_array();

    // simple sanity checks (not strict, only to use values)
    EXPECT_EQ(std::get<0>(tup), 1.f);
    EXPECT_TRUE(det2 != 0.f || inv2 == std::nullopt);
}
