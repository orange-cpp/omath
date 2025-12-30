//
// Created by Vladislav on 30.12.2025.
//
// /Users/vladislav/Downloads/valencia
#include <gtest/gtest.h>
#include <omath/utility/elf_pattern_scan.hpp>

TEST(unit_test_elf_pattern_scan_file, ScanMissingPattern)
{
    constexpr std::string_view path = "/Users/vladislav/Downloads/valencia";

    const auto res = omath::ElfPatternScanner::scan_for_pattern_in_file(path, "FF EE DD", ".text");
    EXPECT_FALSE(res.has_value());
}
