//
// Created by vlad on 5/18/2024.
//
#include <gtest/gtest.h>
#include <omath/Matrix.h>
#include "omath/Vector3.h"

using namespace omath;


class UnitTestMatrix : public ::testing::Test
{
protected:
    Matrix m1;
    Matrix m2;

    void SetUp() override
    {
        m1 = Matrix(2, 2);
        m2 = Matrix{{1.0f, 2.0f}, {3.0f, 4.0f}};
    }
};

// Test constructors
TEST_F(UnitTestMatrix, Constructor_Size)
{
    const Matrix m(3, 3);
    EXPECT_EQ(m.RowCount(), 3);
    EXPECT_EQ(m.ColumnsCount(), 3);
}

TEST_F(UnitTestMatrix, Constructor_InitializerList)
{
    Matrix m{{1.0f, 2.0f}, {3.0f, 4.0f}};
    EXPECT_EQ(m.RowCount(), 2);
    EXPECT_EQ(m.ColumnsCount(), 2);
    EXPECT_FLOAT_EQ(m.At(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(m.At(1, 1), 4.0f);
}

TEST_F(UnitTestMatrix, Constructor_Copy)
{
    Matrix m3 = m2;
    EXPECT_EQ(m3.RowCount(), m2.RowCount());
    EXPECT_EQ(m3.ColumnsCount(), m2.ColumnsCount());
    EXPECT_FLOAT_EQ(m3.At(0, 0), m2.At(0, 0));
}

TEST_F(UnitTestMatrix, Constructor_Move)
{
    Matrix m3 = std::move(m2);
    EXPECT_EQ(m3.RowCount(), 2);
    EXPECT_EQ(m3.ColumnsCount(), 2);
    EXPECT_FLOAT_EQ(m3.At(0, 0), 1.0f);
    EXPECT_EQ(m2.RowCount(), 0);  // m2 should be empty after the move
    EXPECT_EQ(m2.ColumnsCount(), 0);
}

// Test matrix operations
TEST_F(UnitTestMatrix, Operator_Multiplication_Matrix)
{
    Matrix m3 = m2 * m2;
    EXPECT_EQ(m3.RowCount(), 2);
    EXPECT_EQ(m3.ColumnsCount(), 2);
    EXPECT_FLOAT_EQ(m3.At(0, 0), 7.0f);
    EXPECT_FLOAT_EQ(m3.At(1, 1), 22.0f);
}

TEST_F(UnitTestMatrix, Operator_Multiplication_Scalar)
{
    Matrix m3 = m2 * 2.0f;
    EXPECT_FLOAT_EQ(m3.At(0, 0), 2.0f);
    EXPECT_FLOAT_EQ(m3.At(1, 1), 8.0f);
}

TEST_F(UnitTestMatrix, Operator_Division_Scalar)
{
    Matrix m3 = m2 / 2.0f;
    EXPECT_FLOAT_EQ(m3.At(0, 0), 0.5f);
    EXPECT_FLOAT_EQ(m3.At(1, 1), 2.0f);
}

// Test matrix functions
TEST_F(UnitTestMatrix, Transpose)
{
    Matrix m3 = m2.Transpose();
    EXPECT_FLOAT_EQ(m3.At(0, 1), 3.0f);
    EXPECT_FLOAT_EQ(m3.At(1, 0), 2.0f);
}

TEST_F(UnitTestMatrix, Determinant)
{
    const float det = m2.Determinant();
    EXPECT_FLOAT_EQ(det, -2.0f);
}

TEST_F(UnitTestMatrix, Minor)
{
    const float minor = m2.Minor(0, 0);
    EXPECT_FLOAT_EQ(minor, 4.0f);
}

TEST_F(UnitTestMatrix, AlgComplement)
{
    const float algComp = m2.AlgComplement(0, 0);
    EXPECT_FLOAT_EQ(algComp, 4.0f);
}

TEST_F(UnitTestMatrix, Strip)
{
    Matrix m3 = m2.Strip(0, 0);
    EXPECT_EQ(m3.RowCount(), 1);
    EXPECT_EQ(m3.ColumnsCount(), 1);
    EXPECT_FLOAT_EQ(m3.At(0, 0), 4.0f);
}

TEST_F(UnitTestMatrix, ProjectionMatrix)
{
    const Matrix proj = Matrix::ProjectionMatrix(45.0f, 1.33f, 0.1f, 100.0f);
    EXPECT_EQ(proj.RowCount(), 4);
    EXPECT_EQ(proj.ColumnsCount(), 4);
    // Further checks on projection matrix elements could be added
}

// Test other member functions
TEST_F(UnitTestMatrix, Set)
{
    m1.Set(3.0f);
    EXPECT_FLOAT_EQ(m1.At(0, 0), 3.0f);
    EXPECT_FLOAT_EQ(m1.At(1, 1), 3.0f);
}

TEST_F(UnitTestMatrix, Sum)
{
    const float sum = m2.Sum();
    EXPECT_FLOAT_EQ(sum, 10.0f);
}

TEST_F(UnitTestMatrix, Clear)
{
    m2.Clear();
    EXPECT_FLOAT_EQ(m2.At(0, 0), 0.0f);
    EXPECT_FLOAT_EQ(m2.At(1, 1), 0.0f);
}

TEST_F(UnitTestMatrix, ToString)
{
    const std::string str = m2.ToSrtring();
    EXPECT_FALSE(str.empty());
}

// Test assignment operators
TEST_F(UnitTestMatrix, AssignmentOperator_Copy)
{
    Matrix m3(2, 2);
    m3 = m2;
    EXPECT_EQ(m3.RowCount(), m2.RowCount());
    EXPECT_EQ(m3.ColumnsCount(), m2.ColumnsCount());
    EXPECT_FLOAT_EQ(m3.At(0, 0), m2.At(0, 0));
}

TEST_F(UnitTestMatrix, AssignmentOperator_Move)
{
    Matrix m3(2, 2);
    m3 = std::move(m2);
    EXPECT_EQ(m3.RowCount(), 2);
    EXPECT_EQ(m3.ColumnsCount(), 2);
    EXPECT_FLOAT_EQ(m3.At(0, 0), 1.0f);
    EXPECT_EQ(m2.RowCount(), 0);  // m2 should be empty after the move
    EXPECT_EQ(m2.ColumnsCount(), 0);
}