// Additional coverage tests for Vector4 and Mat
#include <gtest/gtest.h>
#include <stdexcept>

#include "omath/linear_algebra/vector4.hpp"
#include "omath/linear_algebra/mat.hpp"

using namespace omath;

static void make_bad_mat_rows()
{
    // wrong number of rows -> should throw inside initializer-list ctor
    Mat<2, 2, float> m{{1.f, 2.f}};
    (void)m;
}

static void make_bad_mat_cols()
{
    // row with wrong number of columns -> should throw
    Mat<2, 2, float> m{{1.f, 2.f}, {1.f}};
    (void)m;
}

TEST(Vector4Operator, Subtraction)
{
    Vector4<float> a{5.f, 6.f, 7.f, 8.f};
    Vector4<float> b{1.f, 2.f, 3.f, 4.f};

    auto r = a - b;
    EXPECT_FLOAT_EQ(r.x, 4.f);
    EXPECT_FLOAT_EQ(r.y, 4.f);
    EXPECT_FLOAT_EQ(r.z, 4.f);
    EXPECT_FLOAT_EQ(r.w, 4.f);
}

TEST(MatInitializerExceptions, ForcedThrowLines)
{
    EXPECT_THROW(make_bad_mat_rows(), std::invalid_argument);
    EXPECT_THROW(make_bad_mat_cols(), std::invalid_argument);
}

TEST(MatSelfAssignment, CopyAndMoveSelfAssign)
{
    Mat<2,2,float> m{{1.f,2.f},{3.f,4.f}};
    // self copy-assignment
    m = m;
    EXPECT_FLOAT_EQ(m.at(0, 0), 1.f);

    // self move-assignment
    m = std::move(m);
    EXPECT_FLOAT_EQ(m.at(0, 0), 1.f);
}
