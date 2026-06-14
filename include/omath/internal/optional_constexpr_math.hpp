//
// Created by orange on 6/11/2026.
//
#pragma once

#include <cmath>
#include <type_traits>

#ifdef OMATH_USE_GCEM
#include <gcem.hpp>
#define OMATH_CONSTEXPR constexpr
#else
#define OMATH_CONSTEXPR
#endif

namespace omath::internal
{
    template<class Type>
    [[nodiscard]]
    OMATH_CONSTEXPR Type sin(const Type& value) noexcept
    {
#ifdef OMATH_USE_GCEM
        if (std::is_constant_evaluated())
            return gcem::sin(value);
#endif
        return std::sin(value);
    }

    template<class Type>
    [[nodiscard]]
    OMATH_CONSTEXPR Type cos(const Type& value) noexcept
    {
#ifdef OMATH_USE_GCEM
        if (std::is_constant_evaluated())
            return gcem::cos(value);
#endif
        return std::cos(value);
    }

    template<class Type>
    [[nodiscard]]
    OMATH_CONSTEXPR Type tan(const Type& value) noexcept
    {
#ifdef OMATH_USE_GCEM
        if (std::is_constant_evaluated())
            return gcem::tan(value);
#endif
        return std::tan(value);
    }

    template<class Type>
    [[nodiscard]]
    OMATH_CONSTEXPR Type atan(const Type& value) noexcept
    {
#ifdef OMATH_USE_GCEM
        if (std::is_constant_evaluated())
            return gcem::atan(value);
#endif
        return std::atan(value);
    }

    template<class Type>
    [[nodiscard]]
    OMATH_CONSTEXPR Type atan2(const Type& y, const Type& x) noexcept
    {
#ifdef OMATH_USE_GCEM
        if (std::is_constant_evaluated())
            return gcem::atan2(y, x);
#endif
        return std::atan2(y, x);
    }

    template<class Type>
    [[nodiscard]]
    OMATH_CONSTEXPR Type asin(const Type& value) noexcept
    {
#ifdef OMATH_USE_GCEM
        if (std::is_constant_evaluated())
            return gcem::asin(value);
#endif
        return std::asin(value);
    }

    template<class Type>
    [[nodiscard]]
    OMATH_CONSTEXPR Type acos(const Type& value) noexcept
    {
#ifdef OMATH_USE_GCEM
        if (std::is_constant_evaluated())
            return gcem::acos(value);
#endif
        return std::acos(value);
    }

    template<class Type>
    [[nodiscard]]
    OMATH_CONSTEXPR Type sqrt(const Type& value) noexcept
    {
#ifdef OMATH_USE_GCEM
        if (std::is_constant_evaluated())
            return gcem::sqrt(value);
#endif
        return std::sqrt(value);
    }

    template<class Type>
    [[nodiscard]]
    OMATH_CONSTEXPR Type hypot(const Type& x, const Type& y) noexcept
    {
#ifdef OMATH_USE_GCEM
        if (std::is_constant_evaluated())
            return gcem::hypot(x, y);
#endif
        return std::hypot(x, y);
    }

    template<class Type>
    [[nodiscard]]
    OMATH_CONSTEXPR Type hypot(const Type& x, const Type& y, const Type& z) noexcept
    {
#ifdef OMATH_USE_GCEM
        if (std::is_constant_evaluated())
            return gcem::sqrt(x * x + y * y + z * z);
#endif
        return std::hypot(x, y, z);
    }

    template<class Type>
    [[nodiscard]]
    OMATH_CONSTEXPR Type abs(const Type& value) noexcept
    {
#ifdef OMATH_USE_GCEM
        if (std::is_constant_evaluated())
            return gcem::abs(value);
#endif
        return std::abs(value);
    }

    template<class Type>
    [[nodiscard]]
    OMATH_CONSTEXPR Type fmod(const Type& dividend, const Type& divisor) noexcept
    {
#ifdef OMATH_USE_GCEM
        if (std::is_constant_evaluated())
            return gcem::fmod(dividend, divisor);
#endif
        return std::fmod(dividend, divisor);
    }
} // namespace omath::internal
