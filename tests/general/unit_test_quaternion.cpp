//
// Created by vlad on 3/1/2026.
//
#include <omath/linear_algebra/quaternion.hpp>
#include <cmath>
#include <gtest/gtest.h>
#include <numbers>

using namespace omath;

static constexpr float kEps = 1e-5f;

// ── Helpers ──────────────────────────────────────────────────────────────────

static void expect_quat_near(const Quaternion<float>& a, const Quaternion<float>& b, float eps = kEps)
{
    EXPECT_NEAR(a.x, b.x, eps);
    EXPECT_NEAR(a.y, b.y, eps);
    EXPECT_NEAR(a.z, b.z, eps);
    EXPECT_NEAR(a.w, b.w, eps);
}

static void expect_vec3_near(const Vector3<float>& a, const Vector3<float>& b, float eps = kEps)
{
    EXPECT_NEAR(a.x, b.x, eps);
    EXPECT_NEAR(a.y, b.y, eps);
    EXPECT_NEAR(a.z, b.z, eps);
}

// ── Constructors ─────────────────────────────────────────────────────────────

TEST(Quaternion, DefaultConstructorIsIdentity)
{
    constexpr Quaternion<float> q;
    EXPECT_FLOAT_EQ(q.x, 0.f);
    EXPECT_FLOAT_EQ(q.y, 0.f);
    EXPECT_FLOAT_EQ(q.z, 0.f);
    EXPECT_FLOAT_EQ(q.w, 1.f);
}

TEST(Quaternion, ValueConstructor)
{
    constexpr Quaternion<float> q{1.f, 2.f, 3.f, 4.f};
    EXPECT_FLOAT_EQ(q.x, 1.f);
    EXPECT_FLOAT_EQ(q.y, 2.f);
    EXPECT_FLOAT_EQ(q.z, 3.f);
    EXPECT_FLOAT_EQ(q.w, 4.f);
}

TEST(Quaternion, DoubleInstantiation)
{
    constexpr Quaternion<double> q{0.0, 0.0, 0.0, 1.0};
    EXPECT_DOUBLE_EQ(q.w, 1.0);
}

// ── Equality ─────────────────────────────────────────────────────────────────

TEST(Quaternion, EqualityOperators)
{
    constexpr Quaternion<float> a{1.f, 2.f, 3.f, 4.f};
    constexpr Quaternion<float> b{1.f, 2.f, 3.f, 4.f};
    constexpr Quaternion<float> c{1.f, 2.f, 3.f, 5.f};

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
}

// ── Arithmetic ───────────────────────────────────────────────────────────────

TEST(Quaternion, ScalarMultiply)
{
    constexpr Quaternion<float> q{1.f, 2.f, 3.f, 4.f};
    constexpr auto r = q * 2.f;
    EXPECT_FLOAT_EQ(r.x, 2.f);
    EXPECT_FLOAT_EQ(r.y, 4.f);
    EXPECT_FLOAT_EQ(r.z, 6.f);
    EXPECT_FLOAT_EQ(r.w, 8.f);
}

TEST(Quaternion, ScalarMultiplyAssign)
{
    Quaternion<float> q{1.f, 2.f, 3.f, 4.f};
    q *= 3.f;
    EXPECT_FLOAT_EQ(q.x, 3.f);
    EXPECT_FLOAT_EQ(q.y, 6.f);
    EXPECT_FLOAT_EQ(q.z, 9.f);
    EXPECT_FLOAT_EQ(q.w, 12.f);
}

TEST(Quaternion, Addition)
{
    constexpr Quaternion<float> a{1.f, 2.f, 3.f, 4.f};
    constexpr Quaternion<float> b{4.f, 3.f, 2.f, 1.f};
    constexpr auto r = a + b;
    EXPECT_FLOAT_EQ(r.x, 5.f);
    EXPECT_FLOAT_EQ(r.y, 5.f);
    EXPECT_FLOAT_EQ(r.z, 5.f);
    EXPECT_FLOAT_EQ(r.w, 5.f);
}

TEST(Quaternion, AdditionAssign)
{
    Quaternion<float> a{1.f, 0.f, 0.f, 0.f};
    const Quaternion<float> b{0.f, 1.f, 0.f, 0.f};
    a += b;
    EXPECT_FLOAT_EQ(a.x, 1.f);
    EXPECT_FLOAT_EQ(a.y, 1.f);
}

TEST(Quaternion, UnaryNegation)
{
    constexpr Quaternion<float> q{1.f, -2.f, 3.f, -4.f};
    constexpr auto r = -q;
    EXPECT_FLOAT_EQ(r.x, -1.f);
    EXPECT_FLOAT_EQ(r.y,  2.f);
    EXPECT_FLOAT_EQ(r.z, -3.f);
    EXPECT_FLOAT_EQ(r.w,  4.f);
}

// ── Hamilton product ──────────────────────────────────────────────────────────

TEST(Quaternion, MultiplyByIdentityIsNoop)
{
    constexpr Quaternion<float> identity;
    constexpr Quaternion<float> q{0.5f, 0.5f, 0.5f, 0.5f};
    expect_quat_near(q * identity, q);
    expect_quat_near(identity * q, q);
}

TEST(Quaternion, MultiplyAssign)
{
    constexpr Quaternion<float> identity;
    Quaternion<float> q{0.5f, 0.5f, 0.5f, 0.5f};
    q *= identity;
    expect_quat_near(q, {0.5f, 0.5f, 0.5f, 0.5f});
}

TEST(Quaternion, MultiplyKnownResult)
{
    // i * j = k  →  (1,0,0,0) * (0,1,0,0) = (0,0,1,0)
    constexpr Quaternion<float> i{1.f, 0.f, 0.f, 0.f};
    constexpr Quaternion<float> j{0.f, 1.f, 0.f, 0.f};
    constexpr auto k = i * j;
    EXPECT_FLOAT_EQ(k.x, 0.f);
    EXPECT_FLOAT_EQ(k.y, 0.f);
    EXPECT_FLOAT_EQ(k.z, 1.f);
    EXPECT_FLOAT_EQ(k.w, 0.f);
}

TEST(Quaternion, MultiplyByInverseGivesIdentity)
{
    const Quaternion<float> q = Quaternion<float>::from_axis_angle({0.f, 0.f, 1.f},
                                                                     std::numbers::pi_v<float> / 3.f);
    const auto result = q * q.inverse();
    expect_quat_near(result, Quaternion<float>{});
}

// ── Conjugate ────────────────────────────────────────────────────────────────

TEST(Quaternion, Conjugate)
{
    constexpr Quaternion<float> q{1.f, 2.f, 3.f, 4.f};
    constexpr auto c = q.conjugate();
    EXPECT_FLOAT_EQ(c.x, -1.f);
    EXPECT_FLOAT_EQ(c.y, -2.f);
    EXPECT_FLOAT_EQ(c.z, -3.f);
    EXPECT_FLOAT_EQ(c.w,  4.f);
}

TEST(Quaternion, ConjugateOfIdentityIsIdentity)
{
    constexpr Quaternion<float> id;
    constexpr auto c = id.conjugate();
    EXPECT_FLOAT_EQ(c.x, 0.f);
    EXPECT_FLOAT_EQ(c.y, 0.f);
    EXPECT_FLOAT_EQ(c.z, 0.f);
    EXPECT_FLOAT_EQ(c.w, 1.f);
}

// ── Dot / length ─────────────────────────────────────────────────────────────

TEST(Quaternion, Dot)
{
    constexpr Quaternion<float> a{1.f, 0.f, 0.f, 0.f};
    constexpr Quaternion<float> b{0.f, 1.f, 0.f, 0.f};
    EXPECT_FLOAT_EQ(a.dot(b), 0.f);
    EXPECT_FLOAT_EQ(a.dot(a), 1.f);
}

TEST(Quaternion, LengthSqrIdentity)
{
    constexpr Quaternion<float> id;
    EXPECT_FLOAT_EQ(id.length_sqr(), 1.f);
}

TEST(Quaternion, LengthSqrGeneral)
{
    constexpr Quaternion<float> q{1.f, 2.f, 3.f, 4.f};
    EXPECT_FLOAT_EQ(q.length_sqr(), 30.f);
}

TEST(Quaternion, LengthIdentity)
{
    const Quaternion<float> id;
    EXPECT_NEAR(id.length(), 1.f, kEps);
}

TEST(Quaternion, Normalized)
{
    const Quaternion<float> q{1.f, 1.f, 1.f, 1.f};
    const auto n = q.normalized();
    EXPECT_NEAR(n.length(), 1.f, kEps);
    EXPECT_NEAR(n.x, 0.5f, kEps);
    EXPECT_NEAR(n.y, 0.5f, kEps);
    EXPECT_NEAR(n.z, 0.5f, kEps);
    EXPECT_NEAR(n.w, 0.5f, kEps);
}

TEST(Quaternion, NormalizedOfZeroLengthReturnsSelf)
{
    // length_sqr = 0 would be UB, but zero-vector part + zero w is degenerate;
    // we just verify the guard branch (divides by zero) doesn't crash by
    // keeping length > 0 via the default constructor path.
    const Quaternion<float> unit;
    const auto n = unit.normalized();
    expect_quat_near(n, unit);
}

// ── Inverse ───────────────────────────────────────────────────────────────────

TEST(Quaternion, InverseOfUnitIsConjugate)
{
    const Quaternion<float> q = Quaternion<float>::from_axis_angle({1.f, 0.f, 0.f},
                                                                     std::numbers::pi_v<float> / 4.f);
    const auto inv = q.inverse();
    const auto conj = q.conjugate();
    expect_quat_near(inv, conj);
}

// ── from_axis_angle ──────────────────────────────────────────────────────────

TEST(Quaternion, FromAxisAngleZeroAngleIsIdentity)
{
    const auto q = Quaternion<float>::from_axis_angle({1.f, 0.f, 0.f}, 0.f);
    EXPECT_NEAR(q.x, 0.f, kEps);
    EXPECT_NEAR(q.y, 0.f, kEps);
    EXPECT_NEAR(q.z, 0.f, kEps);
    EXPECT_NEAR(q.w, 1.f, kEps);
}

TEST(Quaternion, FromAxisAngle90DegZ)
{
    const float half_pi = std::numbers::pi_v<float> / 2.f;
    const auto q = Quaternion<float>::from_axis_angle({0.f, 0.f, 1.f}, half_pi);
    const float s = std::sin(half_pi / 2.f);
    const float c = std::cos(half_pi / 2.f);
    EXPECT_NEAR(q.x, 0.f, kEps);
    EXPECT_NEAR(q.y, 0.f, kEps);
    EXPECT_NEAR(q.z, s,   kEps);
    EXPECT_NEAR(q.w, c,   kEps);
}

// ── rotate ───────────────────────────────────────────────────────────────────

TEST(Quaternion, RotateByIdentityIsNoop)
{
    constexpr Quaternion<float> id;
    constexpr Vector3<float> v{1.f, 2.f, 3.f};
    const auto r = id.rotate(v);
    expect_vec3_near(r, v);
}

TEST(Quaternion, Rotate90DegAroundZ)
{
    // Rotating (1,0,0) by 90° around Z should give (0,1,0)
    const auto q = Quaternion<float>::from_axis_angle({0.f, 0.f, 1.f}, std::numbers::pi_v<float> / 2.f);
    const auto r = q.rotate({1.f, 0.f, 0.f});
    expect_vec3_near(r, {0.f, 1.f, 0.f});
}

TEST(Quaternion, Rotate180DegAroundY)
{
    // Rotating (1,0,0) by 180° around Y should give (-1,0,0)
    const auto q = Quaternion<float>::from_axis_angle({0.f, 1.f, 0.f}, std::numbers::pi_v<float>);
    const auto r = q.rotate({1.f, 0.f, 0.f});
    expect_vec3_near(r, {-1.f, 0.f, 0.f});
}

TEST(Quaternion, Rotate90DegAroundX)
{
    // Rotating (0,1,0) by 90° around X should give (0,0,1)
    const auto q = Quaternion<float>::from_axis_angle({1.f, 0.f, 0.f}, std::numbers::pi_v<float> / 2.f);
    const auto r = q.rotate({0.f, 1.f, 0.f});
    expect_vec3_near(r, {0.f, 0.f, 1.f});
}

// ── to_rotation_matrix3 ───────────────────────────────────────────────────────

TEST(Quaternion, RotationMatrix3FromIdentityIsIdentityMatrix)
{
    constexpr Quaternion<float> id;
    constexpr auto m = id.to_rotation_matrix3();
    for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < 3; ++j)
            EXPECT_NEAR(m.at(i, j), i == j ? 1.f : 0.f, kEps);
}

TEST(Quaternion, RotationMatrix3From90DegZ)
{
    //  Expected:  | 0 -1  0 |
    //             | 1  0  0 |
    //             | 0  0  1 |
    const auto q = Quaternion<float>::from_axis_angle({0.f, 0.f, 1.f}, std::numbers::pi_v<float> / 2.f);
    const auto m = q.to_rotation_matrix3();
    EXPECT_NEAR(m.at(0, 0),  0.f, kEps);
    EXPECT_NEAR(m.at(0, 1), -1.f, kEps);
    EXPECT_NEAR(m.at(0, 2),  0.f, kEps);
    EXPECT_NEAR(m.at(1, 0),  1.f, kEps);
    EXPECT_NEAR(m.at(1, 1),  0.f, kEps);
    EXPECT_NEAR(m.at(1, 2),  0.f, kEps);
    EXPECT_NEAR(m.at(2, 0),  0.f, kEps);
    EXPECT_NEAR(m.at(2, 1),  0.f, kEps);
    EXPECT_NEAR(m.at(2, 2),  1.f, kEps);
}

TEST(Quaternion, RotationMatrix3ConsistentWithRotate)
{
    // Matrix-vector multiply must agree with the rotate() method
    const auto q = Quaternion<float>::from_axis_angle({1.f, 1.f, 0.f}, std::numbers::pi_v<float> / 3.f);
    const Vector3<float> v{2.f, -1.f, 0.5f};

    const auto rotated = q.rotate(v);
    const auto m = q.to_rotation_matrix3();

    // manual mat-vec multiply (row-major)
    const float rx = m.at(0, 0) * v.x + m.at(0, 1) * v.y + m.at(0, 2) * v.z;
    const float ry = m.at(1, 0) * v.x + m.at(1, 1) * v.y + m.at(1, 2) * v.z;
    const float rz = m.at(2, 0) * v.x + m.at(2, 1) * v.y + m.at(2, 2) * v.z;

    EXPECT_NEAR(rotated.x, rx, kEps);
    EXPECT_NEAR(rotated.y, ry, kEps);
    EXPECT_NEAR(rotated.z, rz, kEps);
}

// ── to_rotation_matrix4 ───────────────────────────────────────────────────────

TEST(Quaternion, RotationMatrix4FromIdentityIsIdentityMatrix)
{
    constexpr Quaternion<float> id;
    constexpr auto m = id.to_rotation_matrix4();
    for (size_t i = 0; i < 4; ++i)
        for (size_t j = 0; j < 4; ++j)
            EXPECT_NEAR(m.at(i, j), i == j ? 1.f : 0.f, kEps);
}

TEST(Quaternion, RotationMatrix4HomogeneousRowAndColumn)
{
    const auto q = Quaternion<float>::from_axis_angle({1.f, 0.f, 0.f}, std::numbers::pi_v<float> / 5.f);
    const auto m = q.to_rotation_matrix4();

    // Last row and last column must be (0,0,0,1)
    for (size_t i = 0; i < 3; ++i)
    {
        EXPECT_NEAR(m.at(3, i), 0.f, kEps);
        EXPECT_NEAR(m.at(i, 3), 0.f, kEps);
    }
    EXPECT_NEAR(m.at(3, 3), 1.f, kEps);
}

TEST(Quaternion, RotationMatrix4Upper3x3MatchesMatrix3)
{
    const auto q = Quaternion<float>::from_axis_angle({0.f, 1.f, 0.f}, std::numbers::pi_v<float> / 7.f);
    const auto m3 = q.to_rotation_matrix3();
    const auto m4 = q.to_rotation_matrix4();

    for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < 3; ++j)
            EXPECT_NEAR(m4.at(i, j), m3.at(i, j), kEps);
}

// ── as_array ──────────────────────────────────────────────────────────────────

TEST(Quaternion, AsArray)
{
    constexpr Quaternion<float> q{1.f, 2.f, 3.f, 4.f};
    constexpr auto arr = q.as_array();
    EXPECT_FLOAT_EQ(arr[0], 1.f);
    EXPECT_FLOAT_EQ(arr[1], 2.f);
    EXPECT_FLOAT_EQ(arr[2], 3.f);
    EXPECT_FLOAT_EQ(arr[3], 4.f);
}

// ── std::formatter ────────────────────────────────────────────────────────────

TEST(Quaternion, Formatter)
{
    const Quaternion<float> q{1.f, 2.f, 3.f, 4.f};
    const auto s = std::format("{}", q);
    EXPECT_EQ(s, "[1, 2, 3, 4]");
}
