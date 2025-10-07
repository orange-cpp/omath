//
// Created by Vlad on 10/4/2025.
//
#include "omath/utility/pe_pattern_scan.hpp"
#include "gtest/gtest.h"
#include <omath/utility/pattern_scan.hpp>
#include <source_location>

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
                    "C:\\Users\\Vlad\\CLionProjects\\l4bhop\\cmake-build\\build\\windows-release\\l4bhop.dll"},
            "4C 8B D1 B8 ? ? ? ? F6 04 25 ? ? ? ? ? 75 ? 0F 05 C3");
    EXPECT_FALSE(result.has_value());
}