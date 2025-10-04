//
// Created by Vlad on 10/4/2025.
//
#include "gtest/gtest.h"
#include <omath/utility/pattern_scan.hpp>
#include <source_location>

TEST(unit_test_pattern_scan, read_test)
{
    std::ignore = omath::PatternScanner::parse_pattern("FF ? ?? E9");
}