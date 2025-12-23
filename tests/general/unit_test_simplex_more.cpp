#include "omath/collision/simplex.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <gtest/gtest.h>

using omath::Vector3;
using Simplex = omath::collision::Simplex<Vector3<float>>;

TEST(SimplexExtra, HandleLine_CollinearProducesPerp)
{
    // a and b placed so ab points roughly same dir as ao and are collinear
    Vector3<float> a{2.f, 0.f, 0.f};
    Vector3<float> b{1.f, 0.f, 0.f};

    Simplex s;
    s = {a, b};

    Vector3<float> dir{0.f, 0.f, 0.f};
    const bool hit = s.handle(dir);

    // Should not report collision for a line simplex
    EXPECT_FALSE(hit);
    // Direction must be finite and not zero
    EXPECT_TRUE(std::isfinite(dir.x));
    EXPECT_TRUE(std::isfinite(dir.y));
    EXPECT_TRUE(std::isfinite(dir.z));
    auto zero = Vector3<float>{0.f, 0.f, 0.f};
    EXPECT_FALSE(dir == zero);

    // Ensure direction is (approximately) perpendicular to ab
    const auto ab = b - a;
    const float dot = dir.dot(ab);
    EXPECT_NEAR(dot, 0.0f, 1e-4f);
}

TEST(SimplexExtra, HandleLine_NonCollinearProducesValidDirection)
{
    Vector3<float> a{2.f, 0.f, 0.f};
    Vector3<float> b{1.f, 1.f, 0.f};

    Simplex s;
    s = {a, b};

    Vector3<float> dir{0.f, 0.f, 0.f};
    const bool hit = s.handle(dir);

    EXPECT_FALSE(hit);
    EXPECT_TRUE(std::isfinite(dir.x));
    EXPECT_TRUE(std::isfinite(dir.y));
    EXPECT_TRUE(std::isfinite(dir.z));
}

TEST(SimplexExtra, HandleTriangle_FlipWinding)
{
    // Construct points where triangle winding will be flipped
    Vector3<float> a{1.f, 0.f, 0.f};
    Vector3<float> b{0.f, 1.f, 0.f};
    Vector3<float> c{0.f, -1.f, 0.f};

    Simplex s;
    s = {a, b, c};

    Vector3<float> dir{0.f, 0.f, 0.f};
    const bool hit = s.handle(dir);

    EXPECT_FALSE(hit);
    EXPECT_TRUE(std::isfinite(dir.x));
    EXPECT_TRUE(std::isfinite(dir.y));
    EXPECT_TRUE(std::isfinite(dir.z));
}

TEST(SimplexExtra, HandleTetrahedron_InsideReturnsTrue)
{
    // Simple tetra that should contain the origin
    Vector3<float> a{1.f, 0.f, 0.f};
    Vector3<float> b{0.f, 1.f, 0.f};
    Vector3<float> c{0.f, 0.f, 1.f};
    Vector3<float> d{-0.2f, -0.2f, -0.2f};

    Simplex s;
    s = {a, b, c, d};

    Vector3<float> dir{0.f, 0.f, 0.f};
    const bool hit = s.handle(dir);

    // If origin is inside, handle_tetrahedron should return true
    EXPECT_TRUE(hit);
}
// Additional sanity tests (avoid reusing Simplex alias above to prevent ambiguity)
TEST(SimplexMore, PushFrontAndAccess)
{
    omath::collision::Simplex<omath::Vector3<float>> s;
    s.push_front(omath::Vector3<float>{1.f,0.f,0.f});
    s.push_front(omath::Vector3<float>{2.f,0.f,0.f});
    s.push_front(omath::Vector3<float>{3.f,0.f,0.f});

    EXPECT_EQ(s.size(), 3u);
    omath::Vector3<float> exp_front{3.f,0.f,0.f};
    omath::Vector3<float> exp_back{1.f,0.f,0.f};
    EXPECT_TRUE(s.front() == exp_front);
    EXPECT_TRUE(s.back() == exp_back);
    auto d = s.data();
    EXPECT_TRUE(d[0] == exp_front);
}

TEST(SimplexMore, ClearAndEmpty)
{
    omath::collision::Simplex<omath::Vector3<float>> s;
    s.push_front(omath::Vector3<float>{1.f,1.f,1.f});
    EXPECT_FALSE(s.empty());
    s.clear();
    EXPECT_TRUE(s.empty());
}

TEST(SimplexMore, HandleLineCollinearProducesPerp)
{
    omath::collision::Simplex<omath::Vector3<float>> s;
    s = { omath::Vector3<float>{2.f,0.f,0.f}, omath::Vector3<float>{1.f,0.f,0.f} };
    omath::Vector3<float> dir{0.f,0.f,0.f};
    const bool res = s.handle(dir);
    EXPECT_FALSE(res);
    EXPECT_GT(dir.length_sqr(), 0.0f);
}

TEST(SimplexMore, HandleTriangleFlipWinding)
{
    constexpr omath::Vector3<float> a{1.f,0.f,0.f};
    constexpr omath::Vector3<float> b{0.f,1.f,0.f};
    constexpr omath::Vector3<float> c{0.f,0.f,1.f};
    omath::collision::Simplex<omath::Vector3<float>> s;
    s = { a, b, c };
    omath::Vector3<float> dir{0.f,0.f,0.f};

    const auto ab = b - a;
    const auto ac = c - a;
    const auto abc = ab.cross(ac);

    const bool res = s.handle(dir);
    EXPECT_FALSE(res);
    const auto expected = -abc;
    EXPECT_NEAR(dir.x, expected.x, 1e-6f);
    EXPECT_NEAR(dir.y, expected.y, 1e-6f);
    EXPECT_NEAR(dir.z, expected.z, 1e-6f);
}

TEST(SimplexMore, HandleTetrahedronInsideTrue)
{
    omath::collision::Simplex<omath::Vector3<float>> s;
    s = { omath::Vector3<float>{1.f,0.f,0.f}, omath::Vector3<float>{0.f,1.f,0.f}, omath::Vector3<float>{0.f,0.f,1.f}, omath::Vector3<float>{-1.f,-1.f,-1.f} };
    omath::Vector3<float> dir{0.f,0.f,0.f};
    const bool inside = s.handle(dir);
    EXPECT_TRUE(inside);
}

TEST(SimplexMore, HandlePointSetsDirection)
{
    omath::collision::Simplex<omath::Vector3<float>> s;
    s = { omath::Vector3<float>{1.f,2.f,3.f} };
    omath::Vector3<float> dir{0.f,0.f,0.f};
    EXPECT_FALSE(s.handle(dir));
    EXPECT_NEAR(dir.x, -1.f, 1e-6f);
    EXPECT_NEAR(dir.y, -2.f, 1e-6f);
    EXPECT_NEAR(dir.z, -3.f, 1e-6f);
}

TEST(SimplexMore, HandleLineReducesToPointWhenAoOpposite)
{
    omath::collision::Simplex<omath::Vector3<float>> s;
    s = { omath::Vector3<float>{1.f,0.f,0.f}, omath::Vector3<float>{2.f,0.f,0.f} };
    omath::Vector3<float> dir{0.f,0.f,0.f};
    EXPECT_FALSE(s.handle(dir));
    EXPECT_EQ(s.size(), 1u);
    EXPECT_NEAR(dir.x, -1.f, 1e-6f);
}
