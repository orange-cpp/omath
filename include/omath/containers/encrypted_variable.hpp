//
// Created by Vladislav on 04.01.2026.
//
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#ifdef _MSC_VER
#define OMATH_FORCEINLINE __forceinline
#else
#define OMATH_FORCEINLINE __attribute__((always_inline)) inline
#endif

namespace omath
{
    template<class T>
    class Anchor;

    consteval std::uint64_t fnv1a_64(const char* s)
    {
        std::uint64_t h = 14695981039346656037ull;
        while (*s)
        {
            h ^= static_cast<unsigned char>(*s++);
            h *= 1099511628211ull;
        }
        return h;
    }

    // SplitMix64 mixer (good quality for seeding / scrambling)
    consteval std::uint64_t splitmix64(std::uint64_t x)
    {
        x += 0x9E3779B97F4A7C15ull;
        x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
        x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
        return x ^ (x >> 31);
    }

    // Choose your policy:
    // - If you want reproducible builds, REMOVE __DATE__/__TIME__.
    // - If you want "different each build", keep them.
    consteval std::uint64_t base_seed()
    {
        std::uint64_t h = 0;
        h ^= fnv1a_64(__FILE__);
        h ^= splitmix64(fnv1a_64(__DATE__));
        h ^= splitmix64(fnv1a_64(__TIME__));
        return splitmix64(h);
    }

    // Produce a "random" 64-bit value for a given stream index (compile-time)
    template<std::uint64_t Stream>
    consteval std::uint64_t rand_u64()
    {
        // Stream is usually __COUNTER__ so each call site differs
        return splitmix64(base_seed() + 0xD1B54A32D192ED03ull * (Stream + 1));
    }

    // Unbiased bounded uniform using Lemire's method (uses 128-bit multiply)
    consteval std::uint64_t bounded_u64(std::uint64_t x, std::uint64_t bound)
    {
        // bound must be > 0
        __uint128_t m = static_cast<__uint128_t>(x) * static_cast<__uint128_t>(bound);
        return static_cast<std::uint64_t>(m >> 64);
    }

    template<std::int64_t Lo, std::int64_t Hi, std::uint64_t Stream>
    consteval std::int64_t rand_uint8t()
    {
        static_assert(Lo <= Hi);
        const std::uint64_t span = static_cast<std::uint64_t>(Hi - Lo) + 1ull;
        const std::uint64_t r = rand_u64<Stream>();
        return static_cast<std::int64_t>(bounded_u64(r, span)) + Lo;
    }
    consteval std::uint64_t rand_u64(std::uint64_t seed, std::uint64_t i)
    {
        return splitmix64(seed + 0xD1B54A32D192ED03ull * (i + 1ull));
    }

    // Convert to int (uses low 32 bits; you can also use high bits if you prefer)
    consteval std::uint8_t rand_uint8t(std::uint64_t seed, std::uint64_t i)
    {
        return static_cast<std::uint8_t>(rand_u64(seed, i)); // narrowing is fine/deterministic
    }

    constexpr std::array<std::uint8_t, 6> create_key()
    {
        std::array<std::uint8_t, 6> key{};

        for (auto& byte : key)
        {
            byte = static_cast<std::uint8_t>(rand_u64<__COUNTER__>());
        }
        return key;
    }
    template<std::size_t N, std::uint64_t Seed, std::size_t... I>
    consteval std::array<std::uint8_t, N> make_array_impl(std::index_sequence<I...>)
    {
        return {rand_uint8t(Seed, static_cast<std::uint64_t>(I))...};
    }

    template<std::size_t N, std::uint64_t Seed>
    consteval std::array<std::uint8_t, N> make_array()
    {
        return make_array_impl<N, Seed>(std::make_index_sequence<N>{});
    }
    template<class T, std::size_t key_size, std::array<std::uint8_t, key_size> key>
    class EncryptedVariable final
    {
        bool m_is_encrypted;
        T m_data;

    public:
        OMATH_FORCEINLINE constexpr explicit EncryptedVariable(const T& data): m_is_encrypted(false), m_data(data)
        {
            encrypt();
        }
        [[nodiscard]] constexpr bool is_encrypted() const
        {
            return m_is_encrypted;
        }
        OMATH_FORCEINLINE constexpr void decrypt()
        {
            if (!m_is_encrypted)
                return;
            std::span bytes{reinterpret_cast<std::uint8_t*>(&m_data), sizeof(m_data)};

            for (size_t i = 0; i < bytes.size(); ++i)
                bytes[i] ^= static_cast<std::uint8_t>(key[i % key.size()] + (i * key_size));
            m_is_encrypted = false;
        }
        OMATH_FORCEINLINE constexpr void encrypt()
        {
            if (m_is_encrypted)
                return;
            std::span bytes{reinterpret_cast<std::uint8_t*>(&m_data), sizeof(m_data)};

            for (size_t i = 0; i < bytes.size(); ++i)
                bytes[i] ^= static_cast<std::uint8_t>(key[i % key.size()] + (i * key_size));
            m_is_encrypted = true;
        }
        OMATH_FORCEINLINE constexpr T& value()
        {
            return m_data;
        }
        OMATH_FORCEINLINE constexpr const T& value() const
        {
            return m_data;
        }
        OMATH_FORCEINLINE ~EncryptedVariable()
        {
            decrypt();
        }
    };
    template<class EncryptedVarType>
    class Anchor
    {
    public:
        OMATH_FORCEINLINE constexpr Anchor(EncryptedVarType& var): m_var(var)
        {
            m_var.decrypt();
        }
        OMATH_FORCEINLINE constexpr ~Anchor()
        {
            m_var.encrypt();
        }

    private:
        EncryptedVarType& m_var;
    };
} // namespace omath

#define CT_RAND_ARRAY_INT(N)                                                                                           \
    (::omath::make_array<(N), (::omath::base_seed() ^ static_cast<std::uint64_t>(__COUNTER__))>())
#define OMATH_DEF_CRYPT_VAR(TYPE, KEY_SIZE) omath::EncryptedVariable<TYPE, KEY_SIZE, CT_RAND_ARRAY_INT(KEY_SIZE)>