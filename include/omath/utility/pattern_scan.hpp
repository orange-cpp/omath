//
// Created by Vlad on 10/4/2025.
//

#pragma once
#include <algorithm>
#include <array>
#include <expected>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

// ReSharper disable CppInconsistentNaming
class unit_test_pattern_scan_read_test_Test;
class unit_test_pattern_scan_corner_case_1_Test;
class unit_test_pattern_scan_corner_case_2_Test;
class unit_test_pattern_scan_corner_case_3_Test;
class unit_test_pattern_scan_corner_case_4_Test;
class unit_test_pattern_scan_consteval_read_test_Test;
class unit_test_pattern_scan_consteval_spacing_and_case_Test;
// ReSharper restore CppInconsistentNaming
namespace omath
{
    enum class PatternScanError
    {
        INVALID_PATTERN_STRING
    };
    class PatternScanner final
    {
        friend unit_test_pattern_scan_read_test_Test;
        friend unit_test_pattern_scan_corner_case_1_Test;
        friend unit_test_pattern_scan_corner_case_2_Test;
        friend unit_test_pattern_scan_corner_case_3_Test;
        friend unit_test_pattern_scan_corner_case_4_Test;
        friend unit_test_pattern_scan_consteval_read_test_Test;
        friend unit_test_pattern_scan_consteval_spacing_and_case_Test;

    public:
        template<std::size_t N>
        struct ConstevalPattern final
        {
            char value[N]{};

            // ReSharper disable once CppNonExplicitConvertingConstructor
            constexpr ConstevalPattern(const char (&text)[N]) // NOLINT(*-explicit-constructor)
            {
                std::ranges::copy(text, value);
            }
        };

        [[nodiscard]]
        static std::span<std::byte>::iterator scan_for_pattern(const std::span<std::byte>& range,
                                                               const std::string_view& pattern);

        [[nodiscard]]
        static std::span<std::byte>::iterator scan_for_pattern(std::span<std::byte>&& range,
                                                               const std::string_view& pattern) = delete;

        template<class IteratorType>
        requires std::input_or_output_iterator<std::remove_cvref_t<IteratorType>>
        static IteratorType scan_for_pattern(const IteratorType& begin, const IteratorType& end,
                                             const std::string_view& pattern)
        {
            const auto parsed_pattern = parse_pattern(pattern);

            if (!parsed_pattern) [[unlikely]]
                return end;

            return scan_for_parsed_pattern(begin, end, parsed_pattern.value());
        }
        template<ConstevalPattern Pattern, class IteratorType>
        requires std::input_or_output_iterator<std::remove_cvref_t<IteratorType>>
        static IteratorType scan_for_pattern(const IteratorType& begin, const IteratorType& end)
        {
            constexpr auto parsed_pattern = parse_pattern<Pattern>();

            return scan_for_parsed_pattern(begin, end, parsed_pattern);
        }

    private:
        template<class IteratorType, class ParsedPattern>
        requires std::input_or_output_iterator<std::remove_cvref_t<IteratorType>>
        static IteratorType scan_for_parsed_pattern(const IteratorType& begin, const IteratorType& end,
                                                    const ParsedPattern& parsed_pattern)
        {
            const auto whole_range_size = static_cast<std::ptrdiff_t>(std::distance(begin, end));

            const auto pattern_size = static_cast<std::ptrdiff_t>(parsed_pattern.size());
            const std::ptrdiff_t scan_size = whole_range_size - pattern_size;

            if (scan_size < 0)
                return end;

            for (std::ptrdiff_t i = 0; i <= scan_size; i++)
            {
                bool found = true;

                for (std::ptrdiff_t j = 0; j < static_cast<std::ptrdiff_t>(parsed_pattern.size()); j++)
                {
                    found = parsed_pattern.at(j) == std::nullopt || parsed_pattern.at(j) == *(begin + i + j);

                    if (!found)
                        break;
                }
                if (found)
                    return begin + i;
            }
            return end;
        }
        [[nodiscard]]
        static std::expected<std::vector<std::optional<std::byte>>, PatternScanError>
        parse_pattern(const std::string_view& pattern_string);

        [[nodiscard]]
        constexpr static bool is_space(const char c)
        {
            return c == ' ' || c == '\t' || c == '\n' || c == '\r';
        }

        [[nodiscard]]
        constexpr static int hex_value(const char c)
        {
            if (c >= '0' && c <= '9')
                return c - '0';
            if (c >= 'A' && c <= 'F')
                return c - 'A' + 10;
            if (c >= 'a' && c <= 'f')
                return c - 'a' + 10;
            return -1;
        }
        template<ConstevalPattern Pattern>
        [[nodiscard]]
        static consteval std::size_t signature_size()
        {
            std::size_t count = 0;
            bool in_token = false;

            for (std::size_t i = 0; i + 1 < sizeof(Pattern.value); ++i)
            {
                if (is_space(Pattern.value[i]))
                {
                    in_token = false;
                }
                else if (!in_token)
                {
                    ++count;
                    in_token = true;
                }
            }

            return count;
        }

        template<ConstevalPattern Pattern>
        static consteval std::array<std::optional<std::byte>, signature_size<Pattern>()> parse_pattern()
        {
            std::array<std::optional<std::byte>, signature_size<Pattern>()> result{};
            std::size_t out = 0;
            std::size_t i = 0;

            while (i + 1 < sizeof(Pattern.value))
            {
                while (i + 1 < sizeof(Pattern.value) && is_space(Pattern.value[i]))
                    ++i;

                const std::size_t token_start = i;

                while (i + 1 < sizeof(Pattern.value) && !is_space(Pattern.value[i]))
                    ++i;

                const std::size_t token_size = i - token_start;

                if (token_size == 0)
                    continue;

                // ReSharper disable once CppTooWideScope
                const bool is_wildcard = (token_size == 1 || token_size == 2) && Pattern.value[token_start] == '?';

                if (is_wildcard)
                {
                    if (token_size == 2 && Pattern.value[token_start + 1] != '?')
                        throw std::logic_error("invalid wildcard token");

                    result[out++] = std::nullopt;
                    continue;
                }

                if (token_size != 2)
                    throw std::logic_error("invalid byte token");

                const int high = hex_value(Pattern.value[token_start]);
                const int low = hex_value(Pattern.value[token_start + 1]);

                if (high < 0 || low < 0)
                    throw std::logic_error("invalid hex byte");

                result[out++] = static_cast<std::byte>((high << 4) | low);
            }

            return result;
        }
    };
} // namespace omath
