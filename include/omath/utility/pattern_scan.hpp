//
// Created by Vlad on 10/4/2025.
//

#pragma once
#include <expected>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

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
        static std::optional<std::span<std::byte>::const_iterator> scan_for_pattern(const std::span<std::byte>& range,
                                                                                    const std::string_view& pattern);

        template<class IteratorType>
        static IteratorType scan_for_pattern(const IteratorType& begin, const IteratorType& end,
                                             const std::string_view& pattern)
        {
            const auto parsed_pattern = parse_pattern(pattern);

            if (!parsed_pattern) [[unlikely]]
                return end;

            const auto whole_range_size = static_cast<std::ptrdiff_t>(std::distance(begin, end));

            const std::ptrdiff_t scan_size = whole_range_size - static_cast<std::ptrdiff_t>(pattern.size());

            for (std::ptrdiff_t i = 0; i < scan_size; i++)
            {
                bool found = true;

                for (std::ptrdiff_t j = 0; j < static_cast<std::ptrdiff_t>(parsed_pattern->size()); j++)
                {
                    found = parsed_pattern->at(j) == std::nullopt || parsed_pattern->at(j) == *(begin + i + j);

                    if (!found)
                        break;
                }
                if (found)
                    return begin + i;
            }
            return end;
        }

    private:
        [[nodiscard]]
        static std::expected<std::vector<std::optional<std::byte>>, PatternScanError>
        parse_pattern(const std::string_view& pattern_string);
    };
} // namespace omath