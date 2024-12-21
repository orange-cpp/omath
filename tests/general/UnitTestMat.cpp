// UnitTestMat.cpp
#include <gtest/gtest.h>
#include "omath/Mat.hpp"
#include "omath/Vector3.hpp"

using namespace omath;

class UnitTestMat : public ::testing::Test
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
TEST_F(UnitTestMat, Constructor_Default)
{
    Mat<3, 3> m;
    EXPECT_EQ(m.RowCount(), 3);
    EXPECT_EQ(m.ColumnsCount(), 3);
    for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < 3; ++j)
            EXPECT_FLOAT_EQ(m.At(i, j), 0.0f);
}

TEST_F(UnitTestMat, Constructor_InitializerList)
{
    constexpr Mat<2, 2> m{{1.0f, 2.0f}, {3.0f, 4.0f}};
    EXPECT_EQ(m.RowCount(), 2);
    EXPECT_EQ(m.ColumnsCount(), 2);
    EXPECT_FLOAT_EQ(m.At(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(m.At(0, 1), 2.0f);
    EXPECT_FLOAT_EQ(m.At(1, 0), 3.0f);
    EXPECT_FLOAT_EQ(m.At(1, 1), 4.0f);
}

TEST_F(UnitTestMat, Operator_SquareBrackets)
{
    EXPECT_EQ((m2[0, 0]), 1.0f);
    EXPECT_EQ((m2[0, 1]), 2.0f);
    EXPECT_EQ((m2[1, 0]), 3.0f);
    EXPECT_EQ((m2[1, 1]), 4.0f);
}

TEST_F(UnitTestMat, Constructor_Copy)
{
    Mat<2, 2> m3 = m2;
    EXPECT_EQ(m3.RowCount(), m2.RowCount());
    EXPECT_EQ(m3.ColumnsCount(), m2.ColumnsCount());
    EXPECT_FLOAT_EQ(m3.At(0, 0), m2.At(0, 0));
    EXPECT_FLOAT_EQ(m3.At(1, 1), m2.At(1, 1));
}

TEST_F(UnitTestMat, Constructor_Move)
{
    Mat<2, 2> m3 = std::move(m2);
    EXPECT_EQ(m3.RowCount(), 2);
    EXPECT_EQ(m3.ColumnsCount(), 2);
    EXPECT_FLOAT_EQ(m3.At(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(m3.At(1, 1), 4.0f);
    // m2 is in a valid but unspecified state after move
}

// Test matrix operations
TEST_F(UnitTestMat, Operator_Multiplication_Matrix)
{
    Mat<2, 2> m3 = m2 * m2;
    EXPECT_EQ(m3.RowCount(), 2);
    EXPECT_EQ(m3.ColumnsCount(), 2);
    EXPECT_FLOAT_EQ(m3.At(0, 0), 7.0f);
    EXPECT_FLOAT_EQ(m3.At(0, 1), 10.0f);
    EXPECT_FLOAT_EQ(m3.At(1, 0), 15.0f);
    EXPECT_FLOAT_EQ(m3.At(1, 1), 22.0f);
}

TEST_F(UnitTestMat, Operator_Multiplication_Scalar)
{
    Mat<2, 2> m3 = m2 * 2.0f;
    EXPECT_FLOAT_EQ(m3.At(0, 0), 2.0f);
    EXPECT_FLOAT_EQ(m3.At(1, 1), 8.0f);
}

TEST_F(UnitTestMat, Operator_Division_Scalar)
{
    Mat<2, 2> m3 = m2 / 2.0f;
    EXPECT_FLOAT_EQ(m3.At(0, 0), 0.5f);
    EXPECT_FLOAT_EQ(m3.At(1, 1), 2.0f);
}

// Test matrix functions
TEST_F(UnitTestMat, Transpose)
{
    Mat<2, 2> m3 = m2.Transposed();
    EXPECT_FLOAT_EQ(m3.At(0, 0), m2.At(0, 0));
    EXPECT_FLOAT_EQ(m3.At(0, 1), m2.At(1, 0));
    EXPECT_FLOAT_EQ(m3.At(1, 0), m2.At(0, 1));
    EXPECT_FLOAT_EQ(m3.At(1, 1), m2.At(1, 1));
}

TEST_F(UnitTestMat, Determinant)
{
    const float det = m2.Determinant();
    EXPECT_FLOAT_EQ(det, -2.0f);
}

TEST_F(UnitTestMat, Sum)
{
    const float sum = m2.Sum();
    EXPECT_FLOAT_EQ(sum, 10.0f);
}

TEST_F(UnitTestMat, Clear)
{
    m2.Clear();
    for (size_t i = 0; i < m2.RowCount(); ++i)
        for (size_t j = 0; j < m2.ColumnsCount(); ++j)
            EXPECT_FLOAT_EQ(m2.At(i, j), 0.0f);
}

TEST_F(UnitTestMat, ToString)
{
    const std::string str = m2.ToString();
    EXPECT_FALSE(str.empty());
    EXPECT_EQ(str, "1 2\n3 4\n");
}

// Test assignment operators
TEST_F(UnitTestMat, AssignmentOperator_Copy)
{
    Mat<2, 2> m3;
    m3 = m2;
    EXPECT_EQ(m3.RowCount(), m2.RowCount());
    EXPECT_EQ(m3.ColumnsCount(), m2.ColumnsCount());
    EXPECT_FLOAT_EQ(m3.At(0, 0), m2.At(0, 0));
}

TEST_F(UnitTestMat, AssignmentOperator_Move)
{
    Mat<2, 2> m3;
    m3 = std::move(m2);
    EXPECT_EQ(m3.RowCount(), 2);
    EXPECT_EQ(m3.ColumnsCount(), 2);
    EXPECT_FLOAT_EQ(m3.At(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(m3.At(1, 1), 4.0f);
    // m2 is in a valid but unspecified state after move
}

// Test static methods
TEST_F(UnitTestMat, StaticMethod_ToScreenMat)
{
    Mat<4, 4> screenMat = Mat<4, 4>::ToScreenMat(800.0f, 600.0f);
    EXPECT_FLOAT_EQ(screenMat.At(0, 0), 400.0f);
    EXPECT_FLOAT_EQ(screenMat.At(1, 1), -300.0f);
    EXPECT_FLOAT_EQ(screenMat.At(3, 0), 400.0f);
    EXPECT_FLOAT_EQ(screenMat.At(3, 1), 300.0f);
    EXPECT_FLOAT_EQ(screenMat.At(3, 3), 1.0f);
}


// Test exception handling in At() method
TEST_F(UnitTestMat, Method_At_OutOfRange)
{
    EXPECT_THROW(std::ignore = m2.At(2, 0), std::out_of_range);
    EXPECT_THROW(std::ignore = m2.At(0, 2), std::out_of_range);
}

// Test Determinant for 3x3 matrix
TEST(UnitTestMatStandalone, Determinant_3x3)
{
    constexpr auto det = Mat<3, 3>{{6, 1, 1}, {4, -2, 5}, {2, 8, 7}}.Determinant();
    EXPECT_FLOAT_EQ(det, -306.0f);
}

// Test Minor for 3x3 matrix
TEST(UnitTestMatStandalone, Minor_3x3)
{
    constexpr Mat<3, 3> m{{3, 0, 2}, {2, 0, -2}, {0, 1, 1}};
    auto minor = m.Minor(0, 0);
    EXPECT_EQ(minor.RowCount(), 2);
    EXPECT_EQ(minor.ColumnsCount(), 2);
    EXPECT_FLOAT_EQ(minor.At(0, 0), 0.0f);
    EXPECT_FLOAT_EQ(minor.At(0, 1), -2.0f);
    EXPECT_FLOAT_EQ(minor.At(1, 0), 1.0f);
    EXPECT_FLOAT_EQ(minor.At(1, 1), 1.0f);
}

// Test Transpose for non-square matrix
TEST(UnitTestMatStandalone, Transpose_NonSquare)
{
    constexpr Mat<2, 3> m{{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}};
    auto transposed = m.Transposed();
    EXPECT_EQ(transposed.RowCount(), 3);
    EXPECT_EQ(transposed.ColumnsCount(), 2);
    EXPECT_FLOAT_EQ(transposed.At(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(transposed.At(1, 0), 2.0f);
    EXPECT_FLOAT_EQ(transposed.At(2, 0), 3.0f);
    EXPECT_FLOAT_EQ(transposed.At(0, 1), 4.0f);
    EXPECT_FLOAT_EQ(transposed.At(1, 1), 5.0f);
    EXPECT_FLOAT_EQ(transposed.At(2, 1), 6.0f);
}
