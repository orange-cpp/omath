//
// Created by Orange on 11/30/2024.
//

#pragma once
#include "omath/angles.hpp"
#include <algorithm>
#include <utility>
#include <format>

namespace omath
{
    enum class AngleFlags
    {
        Normalized = 0,
        Clamped = 1,
    };

    template<class Type = float, Type min = Type(0), Type max = Type(360), AngleFlags flags = AngleFlags::Normalized>
    requires std::is_arithmetic_v<Type>
    class Angle
    {
        Type m_angle;
        constexpr explicit Angle(const Type& degrees) noexcept
        {
            if constexpr (flags == AngleFlags::Normalized)
                m_angle = angles::wrap_angle(degrees, min, max);

            else if constexpr (flags == AngleFlags::Clamped)
                m_angle = std::clamp(degrees, min, max);
            else
            {
                static_assert(false);
                std::unreachable();
            }
        }

    public:
        [[nodiscard]]
        constexpr static Angle from_degrees(const Type& degrees) noexcept
        {
            return Angle{degrees};
        }
        constexpr Angle() noexcept: m_angle(0)
        {
        }
        [[nodiscard]]
        constexpr static Angle from_radians(const Type& degrees) noexcept
        {
            return Angle{angles::radians_to_degrees<Type>(degrees)};
        }

        [[nodiscard]]
        constexpr const Type& operator*() const noexcept
        {
            return m_angle;
        }

        [[nodiscard]]
        constexpr Type as_degrees() const noexcept
        {
            return m_angle;
        }

        [[nodiscard]]
        constexpr Type as_radians() const noexcept
        {
            return angles::degrees_to_radians(m_angle);
        }

        [[nodiscard]]
        Type sin() const noexcept
        {
            return std::sin(as_radians());
        }

        [[nodiscard]]
        Type cos() const noexcept
        {
            return std::cos(as_radians());
        }

        [[nodiscard]]
        Type tan() const noexcept
        {
            return std::tan(as_radians());
        }

        [[nodiscard]]
        Type atan() const noexcept
        {
            return std::atan(as_radians());
        }

        [[nodiscard]]
        Type cot() const noexcept
        {
            return cos() / sin();
        }

        constexpr Angle& operator+=(const Angle& other) noexcept
        {
            if constexpr (flags == AngleFlags::Normalized)
                m_angle = angles::wrap_angle(m_angle + other.m_angle, min, max);

            else if constexpr (flags == AngleFlags::Clamped)
                m_angle = std::clamp(m_angle + other.m_angle, min, max);
            else
            {
                static_assert(false);
                std::unreachable();
            }

            return *this;
        }

        [[nodiscard]]
        constexpr std::partial_ordering operator<=>(const Angle& other) const noexcept = default;

        constexpr Angle& operator-=(const Angle& other) noexcept
        {
            return operator+=(-other);
        }

        [[nodiscard]]
        constexpr Angle& operator+(const Angle& other) noexcept
        {
            if constexpr (flags == AngleFlags::Normalized)
                return {angles::wrap_angle(m_angle + other.m_angle, min, max)};

            else if constexpr (flags == AngleFlags::Clamped)
                return {std::clamp(m_angle + other.m_angle, min, max)};

            else
                static_assert(false);

            std::unreachable();
        }

        [[nodiscard]]
        constexpr Angle& operator-(const Angle& other) noexcept
        {
            return operator+(-other);
        }

        [[nodiscard]]
        constexpr Angle operator-() const noexcept
        {
            return Angle{-m_angle};
        }
    };
} // namespace omath
template<class Type, Type min, Type max, omath::AngleFlags flags>
struct std::formatter<omath::Angle<Type, min, max, flags>> // NOLINT(*-dcl58-cpp)
{
    [[nodiscard]]
    static constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }
    [[nodiscard]]
    static auto format(const omath::Angle<Type, min, max, flags>& deg, std::format_context& ctx)
    {
        return std::format_to(ctx.out(), "{}deg", deg.as_degrees());
    }
};