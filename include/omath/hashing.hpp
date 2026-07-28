#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace omath::hashing
{
    enum class Base64Error
    {
        INVALID_LENGTH,
        INVALID_CHARACTER,
        INVALID_PADDING
    };

    [[nodiscard("FNV-1a hash result should not be discarded")]]
    constexpr std::uint64_t fnv1a(const std::string_view value) noexcept
    {
        std::uint64_t hash = 14695981039346656037ULL;

        for (const char character : value)
        {
            hash ^= static_cast<std::uint8_t>(character);
            hash *= 1099511628211ULL;
        }

        return hash;
    }

    [[nodiscard("CRC-32 checksum result should not be discarded")]]
    constexpr std::uint32_t crc32(const std::string_view value) noexcept
    {
        std::uint32_t crc = 0xFFFFFFFFU;

        for (const char character : value)
        {
            crc ^= static_cast<std::uint8_t>(character);

            for (int bit = 0; bit < 8; bit++)
            {
                crc = crc & 1U ? (crc >> 1U) ^ 0xEDB88320U : crc >> 1U;
            }
        }

        return crc ^ 0xFFFFFFFFU;
    }

    [[nodiscard("Base64 encoding result should not be discarded")]]
    constexpr std::string base64_encode(const std::string_view value)
    {
        constexpr std::string_view alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string result;
        result.reserve((value.size() + 2) / 3 * 4);

        for (std::size_t index = 0; index < value.size(); index += 3)
        {
            const std::uint32_t first = static_cast<std::uint8_t>(value[index]);
            const std::uint32_t second = index + 1 < value.size() ? static_cast<std::uint8_t>(value[index + 1]) : 0;
            const std::uint32_t third = index + 2 < value.size() ? static_cast<std::uint8_t>(value[index + 2]) : 0;
            const std::uint32_t chunk = first << 16U | second << 8U | third;

            result += alphabet[(chunk >> 18U) & 0x3FU];
            result += alphabet[(chunk >> 12U) & 0x3FU];
            result += index + 1 < value.size() ? alphabet[(chunk >> 6U) & 0x3FU] : '=';
            result += index + 2 < value.size() ? alphabet[chunk & 0x3FU] : '=';
        }

        return result;
    }

    [[nodiscard("Base64 decoding result should not be discarded")]]
    constexpr std::expected<std::string, Base64Error> base64_decode(const std::string_view value)
    {
        if (value.size() % 4 != 0)
            return std::unexpected(Base64Error::INVALID_LENGTH);

        std::string result;
        result.reserve(value.size() / 4 * 3);

        for (std::size_t index = 0; index < value.size(); index += 4)
        {
            const auto decode_character = [](const char character) constexpr -> int
            {
                if (character >= 'A' && character <= 'Z')
                    return character - 'A';
                if (character >= 'a' && character <= 'z')
                    return character - 'a' + 26;
                if (character >= '0' && character <= '9')
                    return character - '0' + 52;
                if (character == '+')
                    return 62;
                if (character == '/')
                    return 63;

                return -1;
            };

            const bool is_last_chunk = index + 4 == value.size();
            const int first = decode_character(value[index]);
            const int second = decode_character(value[index + 1]);
            const int third = value[index + 2] == '=' ? -2 : decode_character(value[index + 2]);
            const int fourth = value[index + 3] == '=' ? -2 : decode_character(value[index + 3]);

            if (first < 0 || second < 0 || third == -1 || fourth == -1)
                return std::unexpected(Base64Error::INVALID_CHARACTER);
            if (third == -2)
            {
                if (fourth != -2 || !is_last_chunk || (second & 0x0F) != 0)
                    return std::unexpected(Base64Error::INVALID_PADDING);
            }
            else if (fourth == -2 && (!is_last_chunk || (third & 0x03) != 0))
                return std::unexpected(Base64Error::INVALID_PADDING);

            const std::uint32_t chunk = static_cast<std::uint32_t>(first) << 18U
                                        | static_cast<std::uint32_t>(second) << 12U
                                        | static_cast<std::uint32_t>(third < 0 ? 0 : third) << 6U
                                        | static_cast<std::uint32_t>(fourth < 0 ? 0 : fourth);

            result += static_cast<char>(chunk >> 16U);
            if (third >= 0)
                result += static_cast<char>(chunk >> 8U);
            if (fourth >= 0)
                result += static_cast<char>(chunk);
        }

        return result;
    }
} // namespace omath::hashing
