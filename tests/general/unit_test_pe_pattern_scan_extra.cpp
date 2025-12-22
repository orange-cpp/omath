// Tests for PePatternScanner basic behavior
#include <gtest/gtest.h>
#include <omath/utility/pe_pattern_scan.hpp>

using namespace omath;

TEST(unit_test_pe_pattern_scan_extra, MissingFileReturnsNull)
{
    const auto res = PePatternScanner::scan_for_pattern_in_file("/non/existent/file.exe", "55 8B EC");
    EXPECT_FALSE(res.has_value());
}
