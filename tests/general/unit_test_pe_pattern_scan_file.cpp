// Unit test for PePatternScanner::scan_for_pattern_in_file using a synthetic PE-like file
#include <gtest/gtest.h>
#include <omath/utility/pe_pattern_scan.hpp>
#include <cstdint>
#include <vector>
#include "mem_fd_helper.hpp"

using namespace omath;

TEST(unit_test_pe_pattern_scan_file, ScanFindsPattern)
{
    const std::vector<std::uint8_t> bytes = {0x55, 0x8B, 0xEC, 0x90, 0x90};
    const auto f = MemFdFile::create(build_minimal_pe(bytes));
    ASSERT_TRUE(f.valid());

    const auto res = PePatternScanner::scan_for_pattern_in_file(f.path(), "55 8B EC", ".text");
    EXPECT_TRUE(res.has_value());
}

TEST(unit_test_pe_pattern_scan_file, ScanMissingPattern)
{
    const std::vector<std::uint8_t> bytes = {0x00, 0x01, 0x02, 0x03};
    const auto f = MemFdFile::create(build_minimal_pe(bytes));
    ASSERT_TRUE(f.valid());

    const auto res = PePatternScanner::scan_for_pattern_in_file(f.path(), "FF EE DD", ".text");
    EXPECT_FALSE(res.has_value());
}
