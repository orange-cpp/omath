//
// Created by Vlad on 10/4/2025.
//
#include "omath/utility/pattern_scan.hpp"
#include <charconv>
#include <cstdint>
#include <ranges>

namespace
{
    [[nodiscard]]
    constexpr bool is_wildcard(const std::string_view& byte_str)
    {
        return byte_str == "?" || byte_str == "??";
    }

    [[nodiscard]]
    constexpr bool invalid_byte_str_size(const std::string_view& byte_str)
    {
        return byte_str.empty() || byte_str.size() >= 3;
    }
}

namespace omath
{

    std::span<std::byte>::iterator
    PatternScanner::scan_for_pattern(const std::span<std::byte>& range, const std::string_view& pattern)
    {
        return scan_for_pattern(range.begin(), range.end(), pattern);
    }
    std::expected<std::vector<std::optional<std::byte>>, PatternScanError>
        PatternScanner::parse_pattern(const std::string_view& pattern_string)
    {
        std::vector<std::optional<std::byte>> pattern;

        for (const auto part : pattern_string | std::views::split(' '))
        {
            const std::string_view byte_str{part.begin(), part.end()};

            if (invalid_byte_str_size(byte_str)) [[unlikely]]
                continue;

            if (is_wildcard(byte_str))
            {
                pattern.emplace_back(std::nullopt);
                continue;
            }

            std::uint8_t value = 0;
            const auto [ptr, ec] = std::from_chars(byte_str.data(),
                                                   byte_str.data() + byte_str.size(),
                                                   value, 16);

            if (static_cast<int>(ec) != 0) [[unlikely]]
                return std::unexpected(PatternScanError::INVALID_PATTERN_STRING);

            pattern.emplace_back(static_cast<std::byte>(value));
        }

        return pattern;
    }
} // namespace omath