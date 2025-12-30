//
// Created by Vladislav on 30.12.2025.
//
// /Users/vladislav/Downloads/valencia
#include <gtest/gtest.h>
#include <omath/utility/elf_pattern_scan.hpp>
#include <print>
TEST(unit_test_elf_pattern_scan_file, ScanMissingPattern)
{
    constexpr std::string_view path = "/Users/vladislav/Downloads/crackme";

    const auto res = omath::ElfPatternScanner::scan_for_pattern_in_file(path, "F3 0F 1E FA 55 48 89 E5 B8 00 00 00 00", ".text");
    EXPECT_TRUE(res.has_value());

    std::println("In virtual mem: 0x{:x}", res->virtual_base_addr+res->target_offset);
}
