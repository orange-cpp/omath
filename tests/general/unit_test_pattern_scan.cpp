//
// Created by Vlad on 10/4/2025.
//
#include "omath/utility/pe_pattern_scan.hpp"
#include "gtest/gtest.h"
#include <omath/utility/pattern_scan.hpp>
#include <print>
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
    EXPECT_FALSE(result.has_value());
}

TEST(unit_test_pattern_scan, consteval_read_test)
{
    constexpr auto result = omath::PatternScanner::parse_pattern<"FF ? ?? E9">();

    static_assert(result.size() == 4);
    static_assert(result[0] == static_cast<std::byte>(0xFF));
    static_assert(result[1] == std::nullopt);
    static_assert(result[2] == std::nullopt);
    static_assert(result[3] == static_cast<std::byte>(0xE9));

    EXPECT_EQ(result[0], static_cast<std::byte>(0xFF));
    EXPECT_EQ(result[1], std::nullopt);
    EXPECT_EQ(result[2], std::nullopt);
    EXPECT_EQ(result[3], static_cast<std::byte>(0xE9));
}

TEST(unit_test_pattern_scan, consteval_spacing_and_case)
{
    constexpr auto result = omath::PatternScanner::parse_pattern<" \tde\nAD\r?? ? ef ">();

    static_assert(result.size() == 5);
    static_assert(result[0] == static_cast<std::byte>(0xDE));
    static_assert(result[1] == static_cast<std::byte>(0xAD));
    static_assert(result[2] == std::nullopt);
    static_assert(result[3] == std::nullopt);
    static_assert(result[4] == static_cast<std::byte>(0xEF));

    EXPECT_EQ(result[0], static_cast<std::byte>(0xDE));
    EXPECT_EQ(result[1], static_cast<std::byte>(0xAD));
    EXPECT_EQ(result[2], std::nullopt);
    EXPECT_EQ(result[3], std::nullopt);
    EXPECT_EQ(result[4], static_cast<std::byte>(0xEF));
}
