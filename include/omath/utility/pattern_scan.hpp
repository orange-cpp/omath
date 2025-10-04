//
// Created by Vlad on 10/4/2025.
//

#pragma once
#include <expected>
#include <optional>
#include <string_view>
#include <vector>

// ReSharper disable once CppInconsistentNaming
class unit_test_pattern_scan_read_test_Test;
class unit_test_pattern_scan_corner_case_1_Test;
class unit_test_pattern_scan_corner_case_2_Test;
class unit_test_pattern_scan_corner_case_3_Test;
namespace omath
{
    enum class PatternScanError
    {
        INVALID_PATTERN_STRING
    };
    class PatternScanner
    {
        friend unit_test_pattern_scan_read_test_Test;
        friend unit_test_pattern_scan_corner_case_1_Test;
        friend unit_test_pattern_scan_corner_case_2_Test;
        friend unit_test_pattern_scan_corner_case_3_Test;
    public:
    private:
        [[nodiscard]]
        static std::expected<std::vector<std::optional<std::byte>>, PatternScanError>
        parse_pattern(const std::string_view& pattern_string);
    };
} // namespace omath