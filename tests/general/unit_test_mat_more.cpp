// Unit tests to exercise Mat extra branches
#include "gtest/gtest.h"
#include "omath/linear_algebra/mat.hpp"

using omath::Mat;

TEST(MatMore, InitListAndMultiply)
{
    Mat<3,3,float> m{{{1.f,2.f,3.f}, {0.f,1.f,4.f}, {5.f,6.f,0.f}}};
    // multiply by scalar and check element
    auto r = m * 1.f;
    EXPECT_EQ(r.at(0,0), m.at(0,0));
    EXPECT_EQ(r.at(1,2), m.at(1,2));
}

TEST(MatMore, Determinant)
{
    Mat<2,2,double> m{{{1.0,2.0},{2.0,4.0}}}; // singular
    double det = m.determinant();
    EXPECT_DOUBLE_EQ(det, 0.0);
}
