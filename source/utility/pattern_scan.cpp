//
// Created by Vlad on 10/4/2025.
//
#include "omath/utility/pattern_scan.hpp"
#include <charconv>

namespace omath
{

    std::optional<std::span<std::byte>::const_iterator>
    PatternScanner::scan_for_pattern(const std::string_view& pattern, const std::span<std::byte>& range)
    {
        const auto parsed_pattern = parse_pattern(pattern);

        if (!parsed_pattern)
            return std::nullopt;

        const std::ptrdiff_t scan_size =
                static_cast<std::ptrdiff_t>(range.size()) - static_cast<std::ptrdiff_t>(pattern.size());

        for (std::ptrdiff_t i = 0; i < scan_size; i++)
        {
            bool found = true;

            for (std::ptrdiff_t j = 0; j < static_cast<std::ptrdiff_t>(parsed_pattern->size()); j++)
            {
                found = parsed_pattern->at(j) == std::nullopt || parsed_pattern->at(j) == *(range.data() + i + j);

                if (!found)
                    break;
            }
            if (found)
                return range.begin() + i;
        }
        return std::nullopt;
    }
    std::expected<std::vector<std::optional<std::byte>>, PatternScanError>
    PatternScanner::parse_pattern(const std::string_view& pattern_string)
    {
        std::vector<std::optional<std::byte>> pattern;

        auto start = pattern_string.cbegin();

        while (start != pattern_string.cend())
        {
            const auto end = std::ranges::find(start, pattern_string.cend(), ' ');

            const auto sting_view_start = std::distance(pattern_string.cbegin(), start);
            const auto sting_view_end = std::distance(start, end);

            const std::string_view byte_str = pattern_string.substr(sting_view_start, sting_view_end);

            if (byte_str.empty())
            {
                start = end != pattern_string.end() ? std::next(end) : end;
                continue;
            }

            if (byte_str == "?" || byte_str == "??")
            {
                pattern.emplace_back(std::nullopt);

                start = end != pattern_string.end() ? std::next(end) : end;
                continue;
            }

            std::uint8_t value = 0;
            // ReSharper disable once CppTooWideScopeInitStatement
            const auto [_, error_code] = std::from_chars(byte_str.data(), byte_str.data() + byte_str.size(), value, 16);

            if (error_code != std::errc{})
                return std::unexpected(PatternScanError::INVALID_PATTERN_STRING);

            pattern.emplace_back(static_cast<std::byte>(value));

            start = end != pattern_string.end() ? std::next(end) : end;
        }
        return pattern;
    }
} // namespace omath