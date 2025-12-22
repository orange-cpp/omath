#include "omath/collision/simplex.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <gtest/gtest.h>

using omath::Vector3;

TEST(SimplexAdditional, RegionACSelectsAC)
{
    // Construct points that force the Region AC branch where ac points toward the origin
    Vector3<float> a{1.f, 0.f, 0.f};
    Vector3<float> b{2.f, 0.f, 0.f};
    Vector3<float> c{0.f, 1.f, 0.f};

    omath::collision::Simplex<omath::Vector3<float>> s;
    s = { a, b, c };

    omath::Vector3<float> dir{0.f, 0.f, 0.f};
    const bool hit = s.handle(dir);

    // Should not report a collision; simplex should reduce to {a, c}
    EXPECT_FALSE(hit);
    EXPECT_EQ(s.size(), 2u);
    EXPECT_TRUE(s[0] == a);
    EXPECT_TRUE(s[1] == c);
    // direction should be finite and non-zero
    EXPECT_TRUE(std::isfinite(dir.x));
    EXPECT_TRUE(std::isfinite(dir.y));
    EXPECT_TRUE(std::isfinite(dir.z));
}

TEST(SimplexAdditional, AbcAboveSetsDirection)
{
    // Choose triangle so abc points roughly toward the origin (abc · ao > 0)
    Vector3<float> a{-1.f, 0.f, 0.f};
    Vector3<float> b{0.f, 1.f, 0.f};
    Vector3<float> c{0.f, 0.f, 1.f};

    omath::collision::Simplex<omath::Vector3<float>> s;
    s = { a, b, c };

    omath::Vector3<float> dir{0.f, 0.f, 0.f};
    const bool hit = s.handle(dir);

    EXPECT_FALSE(hit);

    const auto ab = b - a;
    const auto ac = c - a;
    const auto abc = ab.cross(ac);

    // direction should equal abc (above triangle case)
    EXPECT_NEAR(dir.x, abc.x, 1e-6f);
    EXPECT_NEAR(dir.y, abc.y, 1e-6f);
    EXPECT_NEAR(dir.z, abc.z, 1e-6f);
}
