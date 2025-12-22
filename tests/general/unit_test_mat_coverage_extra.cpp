// Added to exercise Mat initializer-list exception branches and determinant fallback
#include <gtest/gtest.h>
#include <omath/linear_algebra/mat.hpp>

using namespace omath;

TEST(MatCoverageExtra, InitListRowsMismatchThrows) {
    // Rows mismatch: provide 3 rows for a 2x2 Mat
    EXPECT_THROW((Mat<2,2>{ {1,2}, {3,4}, {5,6} }), std::invalid_argument);
}

TEST(MatCoverageExtra, InitListColumnsMismatchThrows) {
    // Columns mismatch: second row has wrong number of columns
    EXPECT_THROW((Mat<2,2>{ {1,2}, {3} }), std::invalid_argument);
}

TEST(MatCoverageExtra, DeterminantFallbackIsCallable) {
    // Call determinant for 1x1 and 2x2 matrices to cover determinant paths
    Mat<1,1> m1{{3.14f}};
    EXPECT_FLOAT_EQ(m1.determinant(), 3.14f);

    Mat<2,2> m2{{{1.0f,2.0f},{3.0f,4.0f}}};
    EXPECT_FLOAT_EQ(m2.determinant(), -2.0f);
}
