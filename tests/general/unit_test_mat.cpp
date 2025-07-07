// UnitTestMat.cpp
#include <gtest/gtest.h>
#include "omath/mat.hpp"
#include "omath/vector3.hpp"

using namespace omath;

class unit_test_mat : public ::testing::Test
{
protected:
    Mat<2, 2> m1;
    Mat<2, 2> m2;

    void SetUp() override
    {
        m1 = Mat<2, 2>();
        m2 = Mat<2, 2>{{1.0f, 2.0f}, {3.0f, 4.0f}};
    }
};

// Test constructors
TEST_F(unit_test_mat, Constructor_Default)
{
    Mat<3, 3> m;
    EXPECT_EQ(m.row_count(), 3);
    EXPECT_EQ(m.columns_count(), 3);
    for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < 3; ++j)
            EXPECT_FLOAT_EQ(m.at(i, j), 0.0f);
}

TEST_F(unit_test_mat, Constructor_InitializerList)
{
    constexpr Mat<2, 2> m{{1.0f, 2.0f}, {3.0f, 4.0f}};
    EXPECT_EQ(m.row_count(), 2);
    EXPECT_EQ(m.columns_count(), 2);
    EXPECT_FLOAT_EQ(m.at(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(m.at(0, 1), 2.0f);
    EXPECT_FLOAT_EQ(m.at(1, 0), 3.0f);
    EXPECT_FLOAT_EQ(m.at(1, 1), 4.0f);
}

TEST_F(unit_test_mat, Operator_SquareBrackets)
{
    EXPECT_EQ((m2[0, 0]), 1.0f);
    EXPECT_EQ((m2[0, 1]), 2.0f);
    EXPECT_EQ((m2[1, 0]), 3.0f);
    EXPECT_EQ((m2[1, 1]), 4.0f);
}

TEST_F(unit_test_mat, Constructor_Copy)
{
    Mat<2, 2> m3 = m2;
    EXPECT_EQ(m3.row_count(), m2.row_count());
    EXPECT_EQ(m3.columns_count(), m2.columns_count());
    EXPECT_FLOAT_EQ(m3.at(0, 0), m2.at(0, 0));
    EXPECT_FLOAT_EQ(m3.at(1, 1), m2.at(1, 1));
}

TEST_F(unit_test_mat, Constructor_Move)
{
    Mat<2, 2> m3 = std::move(m2);
    EXPECT_EQ(m3.row_count(), 2);
    EXPECT_EQ(m3.columns_count(), 2);
    EXPECT_FLOAT_EQ(m3.at(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(m3.at(1, 1), 4.0f);
    // m2 is in a valid but unspecified state after move
}

// Test matrix operations
TEST_F(unit_test_mat, Operator_Multiplication_Matrix)
{
    Mat<2, 2> m3 = m2 * m2;
    EXPECT_EQ(m3.row_count(), 2);
    EXPECT_EQ(m3.columns_count(), 2);
    EXPECT_FLOAT_EQ(m3.at(0, 0), 7.0f);
    EXPECT_FLOAT_EQ(m3.at(0, 1), 10.0f);
    EXPECT_FLOAT_EQ(m3.at(1, 0), 15.0f);
    EXPECT_FLOAT_EQ(m3.at(1, 1), 22.0f);
}

TEST_F(unit_test_mat, Operator_Multiplication_Scalar)
{
    Mat<2, 2> m3 = m2 * 2.0f;
    EXPECT_FLOAT_EQ(m3.at(0, 0), 2.0f);
    EXPECT_FLOAT_EQ(m3.at(1, 1), 8.0f);
}

TEST_F(unit_test_mat, Operator_Division_Scalar)
{
    Mat<2, 2> m3 = m2 / 2.0f;
    EXPECT_FLOAT_EQ(m3.at(0, 0), 0.5f);
    EXPECT_FLOAT_EQ(m3.at(1, 1), 2.0f);
}

// Test matrix functions
TEST_F(unit_test_mat, Transpose)
{
    Mat<2, 2> m3 = m2.transposed();
    EXPECT_FLOAT_EQ(m3.at(0, 0), m2.at(0, 0));
    EXPECT_FLOAT_EQ(m3.at(0, 1), m2.at(1, 0));
    EXPECT_FLOAT_EQ(m3.at(1, 0), m2.at(0, 1));
    EXPECT_FLOAT_EQ(m3.at(1, 1), m2.at(1, 1));
}

TEST_F(unit_test_mat, Determinant)
{
    const float det = m2.determinant();
    EXPECT_FLOAT_EQ(det, -2.0f);
}

TEST_F(unit_test_mat, Sum)
{
    const float sum = m2.sum();
    EXPECT_FLOAT_EQ(sum, 10.0f);
}

TEST_F(unit_test_mat, Clear)
{
    m2.clear();
    for (size_t i = 0; i < m2.row_count(); ++i)
        for (size_t j = 0; j < m2.columns_count(); ++j)
            EXPECT_FLOAT_EQ(m2.at(i, j), 0.0f);
}

TEST_F(unit_test_mat, ToString)
{
    const std::string str = m2.to_string();
    EXPECT_FALSE(str.empty());
    EXPECT_EQ(str, "[[    1.000,     2.000]\n [    3.000,     4.000]]");
}

// Test assignment operators
TEST_F(unit_test_mat, AssignmentOperator_Copy)
{
    Mat<2, 2> m3;
    m3 = m2;
    EXPECT_EQ(m3.row_count(), m2.row_count());
    EXPECT_EQ(m3.columns_count(), m2.columns_count());
    EXPECT_FLOAT_EQ(m3.at(0, 0), m2.at(0, 0));
}

TEST_F(unit_test_mat, AssignmentOperator_Move)
{
    Mat<2, 2> m3;
    m3 = std::move(m2);
    EXPECT_EQ(m3.row_count(), 2);
    EXPECT_EQ(m3.columns_count(), 2);
    EXPECT_FLOAT_EQ(m3.at(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(m3.at(1, 1), 4.0f);
    // m2 is in a valid but unspecified state after move
}

// Test static methods
TEST_F(unit_test_mat, StaticMethod_ToScreenMat)
{
    Mat<4, 4> screenMat = Mat<4, 4>::to_screen_mat(800.0f, 600.0f);
    EXPECT_FLOAT_EQ(screenMat.at(0, 0), 400.0f);
    EXPECT_FLOAT_EQ(screenMat.at(1, 1), -300.0f);
    EXPECT_FLOAT_EQ(screenMat.at(3, 0), 400.0f);
    EXPECT_FLOAT_EQ(screenMat.at(3, 1), 300.0f);
    EXPECT_FLOAT_EQ(screenMat.at(3, 3), 1.0f);
}


// Test exception handling in At() method
TEST_F(unit_test_mat, Method_At_OutOfRange)
{
#if !defined(NDEBUG) && defined(OMATH_SUPRESS_SAFETY_CHECKS)
    EXPECT_THROW(std::ignore = m2.at(2, 0), std::out_of_range);
    EXPECT_THROW(std::ignore = m2.at(0, 2), std::out_of_range);
#endif
}

// Test Determinant for 3x3 matrix
TEST(UnitTestMatStandalone, Determinant_3x3)
{
    constexpr auto det = Mat<3, 3>{{6, 1, 1}, {4, -2, 5}, {2, 8, 7}}.determinant();
    EXPECT_FLOAT_EQ(det, -306.0f);
}

// Test Minor for 3x3 matrix
TEST(UnitTestMatStandalone, Strip_3x3)
{
    constexpr Mat<3, 3> m{{3, 0, 2}, {2, 0, -2}, {0, 1, 1}};
    auto minor = m.strip(0, 0);
    EXPECT_EQ(minor.row_count(), 2);
    EXPECT_EQ(minor.columns_count(), 2);
    EXPECT_FLOAT_EQ(minor.at(0, 0), 0.0f);
    EXPECT_FLOAT_EQ(minor.at(0, 1), -2.0f);
    EXPECT_FLOAT_EQ(minor.at(1, 0), 1.0f);
    EXPECT_FLOAT_EQ(minor.at(1, 1), 1.0f);
}

// Test Transpose for non-square matrix
TEST(UnitTestMatStandalone, Transpose_NonSquare)
{
    constexpr Mat<2, 3> m{{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}};
    auto transposed = m.transposed();
    EXPECT_EQ(transposed.row_count(), 3);
    EXPECT_EQ(transposed.columns_count(), 2);
    EXPECT_FLOAT_EQ(transposed.at(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(transposed.at(1, 0), 2.0f);
    EXPECT_FLOAT_EQ(transposed.at(2, 0), 3.0f);
    EXPECT_FLOAT_EQ(transposed.at(0, 1), 4.0f);
    EXPECT_FLOAT_EQ(transposed.at(1, 1), 5.0f);
    EXPECT_FLOAT_EQ(transposed.at(2, 1), 6.0f);
}

TEST(UnitTestMatStandalone, Enverse)
{
    constexpr Mat<2, 2> m{{1.0f, 3.0f}, {2.0f, 5.0f}};
    constexpr Mat<2,2> mv{{-5.0f, 3.0f}, {2.0f, -1.0f}};

    EXPECT_EQ(mv, m.inverted());
}
