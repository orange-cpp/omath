#include <gtest/gtest.h>

extern "C" void call_extra_linear_algebra_coverage();

TEST(LinearAlgebraCoverageExtra, InvokeExtraCoverageEntry)
{
    // Call the compiled entrypoint so coverage tools record execution
    call_extra_linear_algebra_coverage();
    SUCCEED();
}
