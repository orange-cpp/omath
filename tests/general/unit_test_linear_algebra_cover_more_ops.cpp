// Added to increase coverage for vector3/vector4/mat headers
#include <gtest/gtest.h>
#include <stdexcept>

#include "omath/linear_algebra/vector3.hpp"
#include "omath/linear_algebra/vector4.hpp"
#include "omath/linear_algebra/mat.hpp"

using namespace omath;

TEST(Vector3ScalarOps, InPlaceScalarOperators)
{
    Vector3<float> v{1.f, 2.f, 3.f};

    v += 1.f;
    EXPECT_FLOAT_EQ(v.x, 2.f);
    EXPECT_FLOAT_EQ(v.y, 3.f);
    EXPECT_FLOAT_EQ(v.z, 4.f);

    v /= 2.f;
    EXPECT_FLOAT_EQ(v.x, 1.f);
    EXPECT_FLOAT_EQ(v.y, 1.5f);
    EXPECT_FLOAT_EQ(v.z, 2.f);

    v -= 0.5f;
    EXPECT_FLOAT_EQ(v.x, 0.5f);
    EXPECT_FLOAT_EQ(v.y, 1.0f);
    EXPECT_FLOAT_EQ(v.z, 1.5f);
}

TEST(Vector4BinaryOps, ElementWiseMulDiv)
{
    constexpr Vector4<float> a{2.f, 4.f, 6.f, 8.f};
    constexpr Vector4<float> b{1.f, 2.f, 3.f, 4.f};

    constexpr auto m = a * b;
    EXPECT_FLOAT_EQ(m.x, 2.f);
    EXPECT_FLOAT_EQ(m.y, 8.f);
    EXPECT_FLOAT_EQ(m.z, 18.f);
    EXPECT_FLOAT_EQ(m.w, 32.f);

    constexpr auto d = a / b;
    EXPECT_FLOAT_EQ(d.x, 2.f);
    EXPECT_FLOAT_EQ(d.y, 2.f);
    EXPECT_FLOAT_EQ(d.z, 2.f);
    EXPECT_FLOAT_EQ(d.w, 2.f);
}

TEST(MatInitExceptions, InvalidInitializerLists)
{
    // Wrong number of rows
    EXPECT_THROW((Mat<2,2,float>{ {1.f,2.f} }), std::invalid_argument);

    // Row with wrong number of columns
    EXPECT_THROW((Mat<2,2,float>{ {1.f,2.f}, {1.f} }), std::invalid_argument);
}
