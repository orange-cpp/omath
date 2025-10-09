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
    EXPECT_FALSE(result.has_value());
}