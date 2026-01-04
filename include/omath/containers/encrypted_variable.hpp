//
// Created by Vladislav on 04.01.2026.
//
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#ifdef OMATH_ENABLE_FORCE_INLINE
#ifdef _MSC_VER
#define OMATH_FORCE_INLINE __forceinline
#else
#define OMATH_FORCE_INLINE __attribute__((always_inline)) inline
#endif
#else
#define OMATH_FORCE_INLINE
#endif

namespace omath::detail
{
    [[nodiscard]]
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
    [[nodiscard]]
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
    [[nodiscard]]
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
    [[nodiscard]]
    consteval std::uint64_t rand_u64()
    {
        // Stream is usually __COUNTER__ so each call site differs
        return splitmix64(base_seed() + 0xD1B54A32D192ED03ull * (Stream + 1));
    }

    [[nodiscard]]
    consteval std::uint64_t bounded_u64(const std::uint64_t x, const std::uint64_t bound)
    {
        return (x * bound) >> 64;
    }

    template<std::int64_t Lo, std::int64_t Hi, std::uint64_t Stream>
    [[nodiscard]]
    consteval std::int64_t rand_uint8_t()
    {
        static_assert(Lo <= Hi);
        const std::uint64_t span = static_cast<std::uint64_t>(Hi - Lo) + 1ull;
        const std::uint64_t r = rand_u64<Stream>();
        return static_cast<std::int64_t>(bounded_u64(r, span)) + Lo;
    }
    [[nodiscard]]
    consteval std::uint64_t rand_u64(const std::uint64_t seed, const std::uint64_t i)
    {
        return splitmix64(seed + 0xD1B54A32D192ED03ull * (i + 1ull));
    }

    // Convert to int (uses low 32 bits; you can also use high bits if you prefer)
    [[nodiscard]]
    consteval std::uint8_t rand_uint8t(const std::uint64_t seed, const std::uint64_t i)
    {
        return static_cast<std::uint8_t>(rand_u64(seed, i)); // narrowing is fine/deterministic
    }
    template<std::size_t N, std::uint64_t Seed, std::size_t... I>
    [[nodiscard]]
    consteval std::array<std::uint8_t, N> make_array_impl(std::index_sequence<I...>)
    {
        return {rand_uint8t(Seed, static_cast<std::uint64_t>(I))...};
    }

    template<std::size_t N, std::uint64_t Seed>
    [[nodiscard]]
    consteval std::array<std::uint8_t, N> make_array()
    {
        return make_array_impl<N, Seed>(std::make_index_sequence<N>{});
    }
} // namespace omath::detail

namespace omath
{
    template<class T>
    class VarAnchor;
    template<class T, std::size_t key_size, std::array<std::uint8_t, key_size> key>
    class EncryptedVariable final
    {
        bool m_is_encrypted;
        T m_data;

        OMATH_FORCE_INLINE constexpr void xor_contained_var_by_key()
        {
            std::span bytes{reinterpret_cast<std::uint8_t*>(&m_data), sizeof(m_data)};

            for (size_t i = 0; i < bytes.size(); ++i)
                bytes[i] ^= static_cast<std::uint8_t>(key[i % key.size()] + (i * key_size));
            m_is_encrypted = true;
        }

    public:
        OMATH_FORCE_INLINE constexpr explicit EncryptedVariable(const T& data): m_is_encrypted(false), m_data(data)
        {
            encrypt();
        }
        [[nodiscard]] constexpr bool is_encrypted() const
        {
            return m_is_encrypted;
        }
        OMATH_FORCE_INLINE constexpr void decrypt()
        {
            if (!m_is_encrypted)
                return;
            xor_contained_var_by_key();
            m_is_encrypted = false;
        }
        OMATH_FORCE_INLINE constexpr void encrypt()
        {
            if (m_is_encrypted)
                return;
            xor_contained_var_by_key();
            m_is_encrypted = true;
        }
        [[nodiscard]]
        OMATH_FORCE_INLINE constexpr T& value()
        {
            return m_data;
        }
        [[nodiscard]]
        OMATH_FORCE_INLINE constexpr const T& value() const
        {
            return m_data;
        }
        OMATH_FORCE_INLINE ~EncryptedVariable()
        {
            decrypt();
        }
        [[nodiscard]]
        OMATH_FORCE_INLINE auto drop_anchor()
        {
            return VarAnchor{*this};
        }
    };
    template<class EncryptedVarType>
    class VarAnchor
    {
    public:
        // ReSharper disable once CppNonExplicitConvertingConstructor
        OMATH_FORCE_INLINE constexpr VarAnchor(EncryptedVarType& var): m_var(var) // NOLINT(*-explicit-constructor)
        {
            m_var.decrypt();
        }
        OMATH_FORCE_INLINE constexpr ~VarAnchor()
        {
            m_var.encrypt();
        }

    private:
        EncryptedVarType& m_var;
    };
} // namespace omath

#define OMATH_CT_RAND_ARRAY_BYTE(N)                                                                                    \
    (::omath::detail::make_array<(N), (::omath::detail::base_seed() ^ static_cast<std::uint64_t>(__COUNTER__))>())
#define OMATH_DEF_CRYPT_VAR(TYPE, KEY_SIZE) omath::EncryptedVariable<TYPE, KEY_SIZE, OMATH_CT_RAND_ARRAY_BYTE(KEY_SIZE)>