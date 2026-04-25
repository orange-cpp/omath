// UnitTestMat.cpp
#include "omath/linear_algebra/mat.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <gtest/gtest.h>

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
    EXPECT_EQ(m.row_count(), 3);
    EXPECT_EQ(m.columns_count(), 3);
    for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < 3; ++j)
            EXPECT_FLOAT_EQ(m.at(i, j), 0.0f);
}

TEST_F(UnitTestMat, Constructor_InitializerList)
{
    constexpr Mat<2, 2> m{{1.0f, 2.0f}, {3.0f, 4.0f}};
    EXPECT_EQ(m.row_count(), 2);
    EXPECT_EQ(m.columns_count(), 2);
    EXPECT_FLOAT_EQ(m.at(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(m.at(0, 1), 2.0f);
    EXPECT_FLOAT_EQ(m.at(1, 0), 3.0f);
    EXPECT_FLOAT_EQ(m.at(1, 1), 4.0f);
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
    EXPECT_EQ(m3.row_count(), m2.row_count());
    EXPECT_EQ(m3.columns_count(), m2.columns_count());
    EXPECT_FLOAT_EQ(m3.at(0, 0), m2.at(0, 0));
    EXPECT_FLOAT_EQ(m3.at(1, 1), m2.at(1, 1));
}

TEST_F(UnitTestMat, Constructor_Move)
{
    Mat<2, 2> m3 = std::move(m2);
    EXPECT_EQ(m3.row_count(), 2);
    EXPECT_EQ(m3.columns_count(), 2);
    EXPECT_FLOAT_EQ(m3.at(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(m3.at(1, 1), 4.0f);
    // m2 is in a valid but unspecified state after move
}

// Test matrix operations
TEST_F(UnitTestMat, Operator_Multiplication_Matrix)
{
    Mat<2, 2> m3 = m2 * m2;
    EXPECT_EQ(m3.row_count(), 2);
    EXPECT_EQ(m3.columns_count(), 2);
    EXPECT_FLOAT_EQ(m3.at(0, 0), 7.0f);
    EXPECT_FLOAT_EQ(m3.at(0, 1), 10.0f);
    EXPECT_FLOAT_EQ(m3.at(1, 0), 15.0f);
    EXPECT_FLOAT_EQ(m3.at(1, 1), 22.0f);
}

TEST_F(UnitTestMat, Operator_Multiplication_Scalar)
{
    Mat<2, 2> m3 = m2 * 2.0f;
    EXPECT_FLOAT_EQ(m3.at(0, 0), 2.0f);
    EXPECT_FLOAT_EQ(m3.at(1, 1), 8.0f);
}

TEST_F(UnitTestMat, Operator_Division_Scalar)
{
    Mat<2, 2> m3 = m2 / 2.0f;
    EXPECT_FLOAT_EQ(m3.at(0, 0), 0.5f);
    EXPECT_FLOAT_EQ(m3.at(1, 1), 2.0f);
}

// Test matrix functions
TEST_F(UnitTestMat, Transpose)
{
    Mat<2, 2> m3 = m2.transposed();
    EXPECT_FLOAT_EQ(m3.at(0, 0), m2.at(0, 0));
    EXPECT_FLOAT_EQ(m3.at(0, 1), m2.at(1, 0));
    EXPECT_FLOAT_EQ(m3.at(1, 0), m2.at(0, 1));
    EXPECT_FLOAT_EQ(m3.at(1, 1), m2.at(1, 1));
}

TEST_F(UnitTestMat, Determinant)
{
    const float det = m2.determinant();
    EXPECT_FLOAT_EQ(det, -2.0f);
}

TEST_F(UnitTestMat, Sum)
{
    const float sum = m2.sum();
    EXPECT_FLOAT_EQ(sum, 10.0f);
}

TEST_F(UnitTestMat, Clear)
{
    m2.clear();
    for (size_t i = 0; i < m2.row_count(); ++i)
        for (size_t j = 0; j < m2.columns_count(); ++j)
            EXPECT_FLOAT_EQ(m2.at(i, j), 0.0f);
}

TEST_F(UnitTestMat, ToString)
{
    const std::string str = std::format("{}", m2);
    EXPECT_FALSE(str.empty());
    EXPECT_EQ(str, "[[    1.000,     2.000]\n [    3.000,     4.000]]");
}

// Test assignment operators
TEST_F(UnitTestMat, AssignmentOperator_Copy)
{
    Mat<2, 2> m3;
    m3 = m2;
    EXPECT_EQ(m3.row_count(), m2.row_count());
    EXPECT_EQ(m3.columns_count(), m2.columns_count());
    EXPECT_FLOAT_EQ(m3.at(0, 0), m2.at(0, 0));
}

TEST_F(UnitTestMat, AssignmentOperator_Move)
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
TEST_F(UnitTestMat, StaticMethod_ToScreenMat)
{
    Mat<4, 4> screen_mat = Mat<4, 4>::to_screen_mat(800.0f, 600.0f);
    EXPECT_FLOAT_EQ(screen_mat.at(0, 0), 400.0f);
    EXPECT_FLOAT_EQ(screen_mat.at(1, 1), -300.0f);
    EXPECT_FLOAT_EQ(screen_mat.at(3, 0), 400.0f);
    EXPECT_FLOAT_EQ(screen_mat.at(3, 1), 300.0f);
    EXPECT_FLOAT_EQ(screen_mat.at(3, 3), 1.0f);
}


// Test exception handling in At() method
TEST_F(UnitTestMat, Method_At_OutOfRange)
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

TEST(UnitTestMatStandalone, Equanity)
{
    constexpr omath::Vector3<float> left_handed = {0, 2, 10};
    constexpr omath::Vector3<float> right_handed = {0, 2, -10};

    const auto proj_left_handed = omath::mat_perspective_left_handed(90.f, 16.f / 9.f, 0.1f, 1000.f);
    const auto proj_right_handed = omath::mat_perspective_right_handed(90.f, 16.f / 9.f, 0.1f, 1000.f);

    auto ndc_left_handed = proj_left_handed * omath::mat_column_from_vector(left_handed);
    auto ndc_right_handed = proj_right_handed * omath::mat_column_from_vector(right_handed);

    ndc_left_handed /= ndc_left_handed.at(3, 0);
    ndc_right_handed /= ndc_right_handed.at(3, 0);

    EXPECT_EQ(ndc_left_handed, ndc_right_handed);
}
TEST(UnitTestMatStandalone, MatPerspectiveLeftHanded)
{
    const auto perspective_proj = mat_perspective_left_handed(90.f, 16.f/9.f, 0.1f, 1000.f);
    auto projected = perspective_proj
                             * mat_column_from_vector<float>({0, 0, 0.1001});

    projected /= projected.at(3, 0);

    EXPECT_TRUE(projected.at(2, 0) > -1.0f && projected.at(2, 0) < 0.f);
}

TEST(UnitTestMatStandalone, MatPerspectiveLeftHandedZeroToOne)
{
    const auto proj = mat_perspective_left_handed<float, MatStoreType::ROW_MAJOR, NDCDepthRange::ZERO_TO_ONE>(
            90.f, 16.f / 9.f, 0.1f, 1000.f);

    // Near plane point should map to z ~ 0
    auto near_pt = proj * mat_column_from_vector<float>({0, 0, 0.1f});
    near_pt /= near_pt.at(3, 0);
    EXPECT_NEAR(near_pt.at(2, 0), 0.0f, 1e-4f);

    // Far plane point should map to z ~ 1
    auto far_pt = proj * mat_column_from_vector<float>({0, 0, 1000.f});
    far_pt /= far_pt.at(3, 0);
    EXPECT_NEAR(far_pt.at(2, 0), 1.0f, 1e-4f);

    // Mid-range point should be in [0, 1]
    auto mid_pt = proj * mat_column_from_vector<float>({0, 0, 500.f});
    mid_pt /= mid_pt.at(3, 0);
    EXPECT_GT(mid_pt.at(2, 0), 0.0f);
    EXPECT_LT(mid_pt.at(2, 0), 1.0f);
}

TEST(UnitTestMatStandalone, MatPerspectiveRightHandedZeroToOne)
{
    const auto proj = mat_perspective_right_handed<float, MatStoreType::ROW_MAJOR, NDCDepthRange::ZERO_TO_ONE>(
            90.f, 16.f / 9.f, 0.1f, 1000.f);

    // Near plane point (negative z for right-handed) should map to z ~ 0
    auto near_pt = proj * mat_column_from_vector<float>({0, 0, -0.1f});
    near_pt /= near_pt.at(3, 0);
    EXPECT_NEAR(near_pt.at(2, 0), 0.0f, 1e-4f);

    // Far plane point should map to z ~ 1
    auto far_pt = proj * mat_column_from_vector<float>({0, 0, -1000.f});
    far_pt /= far_pt.at(3, 0);
    EXPECT_NEAR(far_pt.at(2, 0), 1.0f, 1e-4f);

    // Mid-range point should be in [0, 1]
    auto mid_pt = proj * mat_column_from_vector<float>({0, 0, -500.f});
    mid_pt /= mid_pt.at(3, 0);
    EXPECT_GT(mid_pt.at(2, 0), 0.0f);
    EXPECT_LT(mid_pt.at(2, 0), 1.0f);
}

TEST(UnitTestMatStandalone, MatPerspectiveNegativeOneToOneRange)
{
    // Verify existing [-1, 1] behavior with explicit template arg matches default
    const auto proj_default = mat_perspective_left_handed(90.f, 16.f / 9.f, 0.1f, 1000.f);
    const auto proj_explicit = mat_perspective_left_handed<float, MatStoreType::ROW_MAJOR,
            NDCDepthRange::NEGATIVE_ONE_TO_ONE>(90.f, 16.f / 9.f, 0.1f, 1000.f);

    EXPECT_EQ(proj_default, proj_explicit);

    // Near plane should map to z ~ -1
    auto near_pt = proj_default * mat_column_from_vector<float>({0, 0, 0.1f});
    near_pt /= near_pt.at(3, 0);
    EXPECT_NEAR(near_pt.at(2, 0), -1.0f, 1e-3f);

    // Far plane should map to z ~ 1
    auto far_pt = proj_default * mat_column_from_vector<float>({0, 0, 1000.f});
    far_pt /= far_pt.at(3, 0);
    EXPECT_NEAR(far_pt.at(2, 0), 1.0f, 1e-3f);
}

TEST(UnitTestMatStandalone, MatPerspectiveZeroToOneEquanity)
{
    // LH and RH should produce same NDC for mirrored z
    constexpr omath::Vector3<float> left_handed = {0, 2, 10};
    constexpr omath::Vector3<float> right_handed = {0, 2, -10};

    const auto proj_lh = mat_perspective_left_handed<float, MatStoreType::ROW_MAJOR, NDCDepthRange::ZERO_TO_ONE>(
            90.f, 16.f / 9.f, 0.1f, 1000.f);
    const auto proj_rh = mat_perspective_right_handed<float, MatStoreType::ROW_MAJOR, NDCDepthRange::ZERO_TO_ONE>(
            90.f, 16.f / 9.f, 0.1f, 1000.f);

    auto ndc_lh = proj_lh * mat_column_from_vector(left_handed);
    auto ndc_rh = proj_rh * mat_column_from_vector(right_handed);

    ndc_lh /= ndc_lh.at(3, 0);
    ndc_rh /= ndc_rh.at(3, 0);

    EXPECT_EQ(ndc_lh, ndc_rh);
}

TEST(UnitTestMatStandalone, MatOrthoLeftHandedZeroToOne)
{
    const auto ortho = mat_ortho_left_handed<float, MatStoreType::ROW_MAJOR, NDCDepthRange::ZERO_TO_ONE>(
            -1.f, 1.f, -1.f, 1.f, 0.1f, 100.f);

    // Near plane should map to z ~ 0
    auto near_pt = ortho * mat_column_from_vector<float>({0, 0, 0.1f});
    EXPECT_NEAR(near_pt.at(2, 0), 0.0f, 1e-4f);

    // Far plane should map to z ~ 1
    auto far_pt = ortho * mat_column_from_vector<float>({0, 0, 100.f});
    EXPECT_NEAR(far_pt.at(2, 0), 1.0f, 1e-4f);
}

TEST(UnitTestMatStandalone, MatOrthoRightHandedZeroToOne)
{
    const auto ortho = mat_ortho_right_handed<float, MatStoreType::ROW_MAJOR, NDCDepthRange::ZERO_TO_ONE>(
            -1.f, 1.f, -1.f, 1.f, 0.1f, 100.f);

    // Near plane (negative z for RH) should map to z ~ 0
    auto near_pt = ortho * mat_column_from_vector<float>({0, 0, -0.1f});
    EXPECT_NEAR(near_pt.at(2, 0), 0.0f, 1e-4f);

    // Far plane should map to z ~ 1
    auto far_pt = ortho * mat_column_from_vector<float>({0, 0, -100.f});
    EXPECT_NEAR(far_pt.at(2, 0), 1.0f, 1e-4f);
}

TEST(UnitTestMatStandalone, MatOrthoNegativeOneToOneDefault)
{
    // Verify explicit [-1, 1] matches default
    const auto ortho_default = mat_ortho_left_handed(-1.f, 1.f, -1.f, 1.f, 0.1f, 100.f);
    const auto ortho_explicit = mat_ortho_left_handed<float, MatStoreType::ROW_MAJOR,
            NDCDepthRange::NEGATIVE_ONE_TO_ONE>(-1.f, 1.f, -1.f, 1.f, 0.1f, 100.f);

    EXPECT_EQ(ortho_default, ortho_explicit);
}