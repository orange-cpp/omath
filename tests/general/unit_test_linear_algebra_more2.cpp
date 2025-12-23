// Tests to exercise non-inlined helpers and remaining branches in linear algebra
#include "gtest/gtest.h"
#include "omath/linear_algebra/vector3.hpp"
#include "omath/linear_algebra/vector4.hpp"
#include "omath/linear_algebra/mat.hpp"

using namespace omath;

TEST(LinearAlgebraMore2, Vector3NonInlinedHelpers)
{
    Vector3<float> v{3.f, 4.f, 0.f};
    EXPECT_FLOAT_EQ(v.length(), 5.0f);

    auto vn = v.normalized();
    EXPECT_NEAR(vn.length(), 1.0f, 1e-6f);

    Vector3<float> zero{0.f,0.f,0.f};
    auto ang = v.angle_between(zero);
    EXPECT_FALSE(ang.has_value());

    Vector3<float> a{1.f,0.f,0.f};
    Vector3<float> b{0.f,1.f,0.f};
    EXPECT_TRUE(a.is_perpendicular(b));
    EXPECT_FALSE(a.is_perpendicular(a));

    auto tup = v.as_tuple();
    EXPECT_EQ(std::get<0>(tup), 3.f);
    EXPECT_EQ(std::get<1>(tup), 4.f);
    EXPECT_EQ(std::get<2>(tup), 0.f);

    EXPECT_TRUE(a.point_to_same_direction(Vector3<float>{2.f,0.f,0.f}));

    // exercise hash specialization for Vector3<float>
    std::hash<Vector3<float>> hasher;
    auto hv = hasher(v);
    (void)hv;
}

TEST(LinearAlgebraMore2, Vector4NonInlinedHelpers)
{
    Vector4<float> v{1.f,2.f,3.f,4.f};
    EXPECT_FLOAT_EQ(v.length(), v.length());
    EXPECT_FLOAT_EQ(v.sum(), v.sum());

    // clamp noinline should modify the vector
    v.clamp(0.f, 2.5f);
    EXPECT_GE(v.x, 0.f);
    EXPECT_LE(v.z, 2.5f);

    const Vector4<float> shorter{0.1f,0.1f,0.1f,0.1f};
    EXPECT_TRUE(shorter < v);
    EXPECT_FALSE(v < shorter);
}

TEST(LinearAlgebraMore2, MatNonInlinedAndStringHelpers)
{
    Mat<2,2,float> m{{{4.f,7.f},{2.f,6.f}}};
    EXPECT_FLOAT_EQ(m.determinant(), 10.0f);

    auto maybe_inv = m.inverted();
    EXPECT_TRUE(maybe_inv.has_value());
    const auto& inv = maybe_inv.value();

    // m * inv should be identity (approximately)
    auto prod = m * inv;
    EXPECT_NEAR(prod.at(0,0), 1.0f, 1e-5f);
    EXPECT_NEAR(prod.at(1,1), 1.0f, 1e-5f);
    EXPECT_NEAR(prod.at(0,1), 0.0f, 1e-5f);

    // transposed and to_string variants
    auto t = m.transposed();
    EXPECT_EQ(t.at(0,1), m.at(1,0));

    auto raw = m.raw_array();
    EXPECT_EQ(raw.size(), static_cast<size_t>(4));

    auto s = m.to_string();
    EXPECT_NE(s.size(), 0u);
    auto ws = m.to_wstring();
    EXPECT_NE(ws.size(), 0u);
    auto u8_s = m.to_u8string();
    EXPECT_NE(u8_s.size(), 0u);

    // to_screen_mat static helper
    auto screen = Mat<4,4,float>::to_screen_mat(800.f, 600.f);
    EXPECT_NEAR(screen.at(0,0), 800.f/2.f, 1e-6f);
}
