#include <gtest/gtest.h>
#include "coverage_wrappers.hpp"

TEST(MatInitWrappers, CallRowsMismatchWrapper) {
    coverage_mat_init_rows_mismatch();
    SUCCEED();
}

TEST(MatInitWrappers, CallColumnsMismatchWrapper) {
    coverage_mat_init_columns_mismatch();
    SUCCEED();
}
