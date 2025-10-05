//
// Created by Vlad on 10/4/2025.
//

#pragma once
#include <expected>
#include <optional>
#include <string_view>
#include <vector>
#include <span>

// ReSharper disable once CppInconsistentNaming
class unit_test_pattern_scan_read_test_Test;
// ReSharper disable once CppInconsistentNaming
class unit_test_pattern_scan_corner_case_1_Test;
// ReSharper disable once CppInconsistentNaming
class unit_test_pattern_scan_corner_case_2_Test;
// ReSharper disable once CppInconsistentNaming
class unit_test_pattern_scan_corner_case_3_Test;
// ReSharper disable once CppInconsistentNaming
class unit_test_pattern_scan_corner_case_4_Test;


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
        friend unit_test_pattern_scan_corner_case_4_Test;
    public:
        [[nodiscard]]
        static std::optional<std::span<std::byte>::const_iterator>
        scan_for_pattern(const std::string_view& pattern, const std::span<std::byte>& range);
    private:
        [[nodiscard]]
        static std::expected<std::vector<std::optional<std::byte>>, PatternScanError>
        parse_pattern(const std::string_view& pattern_string);
    };
} // namespace omath