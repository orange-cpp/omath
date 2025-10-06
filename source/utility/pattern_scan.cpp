//
// Created by Vlad on 10/4/2025.
//
#include "omath/utility/pattern_scan.hpp"
#include <charconv>
#include <cstdint>
namespace omath
{

    std::span<std::byte>::const_iterator
    PatternScanner::scan_for_pattern(const std::span<std::byte>& range, const std::string_view& pattern)
    {
        return scan_for_pattern(range.begin(), range.end(), pattern);
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

            if (byte_str.empty()) [[unlikely]]
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

            if (error_code != std::errc{}) [[unlikely]]
                return std::unexpected(PatternScanError::INVALID_PATTERN_STRING);

            pattern.emplace_back(static_cast<std::byte>(value));

            start = end != pattern_string.end() ? std::next(end) : end;
        }
        return pattern;
    }
} // namespace omath