//
// Created by Vlad on 10/4/2025.
//
#include "omath/utility/pe_pattern_scan.hpp"
#include "gtest/gtest.h"
#include <omath/utility/pattern_scan.hpp>
#include <source_location>
#include <print>
TEST(unit_test_pattern_scan, read_test)
{
    const auto result = omath::PatternScanner::parse_pattern("FF ? ?? E9");

    EXPECT_EQ(result->at(0), static_cast<std::byte>(0xFF));
    EXPECT_EQ(result->at(1), std::nullopt);
    EXPECT_EQ(result->at(2), std::nullopt);
    EXPECT_EQ(result->at(3), static_cast<std::byte>(0xE9));
}

TEST(unit_test_pattern_scan, corner_case_1)
{
    const auto result = omath::PatternScanner::parse_pattern("     FF ? ?? E9");

    EXPECT_EQ(result->at(0), static_cast<std::byte>(0xFF));
    EXPECT_EQ(result->at(1), std::nullopt);
    EXPECT_EQ(result->at(2), std::nullopt);
    EXPECT_EQ(result->at(3), static_cast<std::byte>(0xE9));
}

TEST(unit_test_pattern_scan, corner_case_2)
{
    const auto result = omath::PatternScanner::parse_pattern("     FF ? ?? E9       ");

    EXPECT_EQ(result->at(0), static_cast<std::byte>(0xFF));
    EXPECT_EQ(result->at(1), std::nullopt);
    EXPECT_EQ(result->at(2), std::nullopt);
    EXPECT_EQ(result->at(3), static_cast<std::byte>(0xE9));
}

TEST(unit_test_pattern_scan, corner_case_3)
{
    const auto result = omath::PatternScanner::parse_pattern("     FF ?       ??      E9       ");

    EXPECT_EQ(result->at(0), static_cast<std::byte>(0xFF));
    EXPECT_EQ(result->at(1), std::nullopt);
    EXPECT_EQ(result->at(2), std::nullopt);
    EXPECT_EQ(result->at(3), static_cast<std::byte>(0xE9));
}

TEST(unit_test_pattern_scan, corner_case_4)
{
    const auto result = omath::PatternScanner::parse_pattern("X ? ?? E9 ");
    const auto result2 = omath::PePatternScanner::scan_for_pattern_in_file(
            std::filesystem::path{
                    R"(C:\Users\Vlad\CLionProjects\aether\out\Release\aether.dll)"},
            "48 89 5C 24 ? 57 48 83 EC ? 8B DA 48 8B F9 FF 15 ? ? ? ? 83 FB ? 75 ? B9 ? ? ? ? E8 ? ? ? ? 33 DB 48 85 C0 74 ? 48 8D 0D ? ? ? ? 48 89 38 48 89 48 ? EB");

    std::println("{:x}", result2->virtual_base_addr + result2->target_offset);
    EXPECT_FALSE(result.has_value());
}