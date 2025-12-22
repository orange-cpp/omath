// Extra tests for PatternScanner behavior
#include <gtest/gtest.h>
#include <omath/utility/pattern_scan.hpp>

using namespace omath;

TEST(unit_test_pattern_scan_extra, IteratorScanFound)
{
    std::vector<std::byte> buf = {std::byte(0xDE), std::byte(0xAD), std::byte(0xBE), std::byte(0xEF), std::byte(0x00)};
    auto it = PatternScanner::scan_for_pattern(buf.begin(), buf.end(), "DE AD BE EF");
    EXPECT_NE(it, buf.end());
    EXPECT_EQ(std::distance(buf.begin(), it), 0);
}

TEST(unit_test_pattern_scan_extra, IteratorScanNotFound)
{
    std::vector<std::byte> buf = {std::byte(0x00), std::byte(0x11), std::byte(0x22)};
    auto it = PatternScanner::scan_for_pattern(buf.begin(), buf.end(), "FF EE DD");
    EXPECT_EQ(it, buf.end());
}

TEST(unit_test_pattern_scan_extra, ParseInvalidPattern)
{
    // invalid hex token should cause the public scan to return end (no match)
    std::vector<std::byte> buf = {std::byte(0x00), std::byte(0x11)};
    auto it = PatternScanner::scan_for_pattern(buf.begin(), buf.end(), "GG HH");
    EXPECT_EQ(it, buf.end());
}
