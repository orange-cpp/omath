//
// Created by Orange on 11/30/2024.
//

#pragma once
#include "omath/angles.hpp"
#include <algorithm>
#include <utility>

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
        constexpr explicit Angle(const Type& degrees)
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
        constexpr static Angle from_degrees(const Type& degrees)
        {
            return Angle{degrees};
        }
        constexpr Angle(): m_angle(0)
        {
        }
        [[nodiscard]]
        constexpr static Angle from_radians(const Type& degrees)
        {
            return Angle{angles::radians_to_degrees<Type>(degrees)};
        }

        [[nodiscard]]
        constexpr const Type& operator*() const
        {
            return m_angle;
        }

        [[nodiscard]]
        constexpr Type as_degrees() const
        {
            return m_angle;
        }

        [[nodiscard]]
        constexpr Type as_radians() const
        {
            return angles::degrees_to_radians(m_angle);
        }

        [[nodiscard]]
        Type sin() const
        {
            return std::sin(as_radians());
        }

        [[nodiscard]]
        Type cos() const
        {
            return std::cos(as_radians());
        }

        [[nodiscard]]
        Type tan() const
        {
            return std::tan(as_radians());
        }

        [[nodiscard]]
        Type atan() const
        {
            return std::atan(as_radians());
        }

        [[nodiscard]]
        Type cot() const
        {
            return cos() / sin();
        }

        constexpr Angle& operator+=(const Angle& other)
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
        constexpr std::partial_ordering operator<=>(const Angle& other) const
                = default;

        constexpr Angle& operator-=(const Angle& other)
        {
            return operator+=(-other);
        }

        [[nodiscard]]
        constexpr Angle& operator+(const Angle& other)
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
        constexpr Angle& operator-(const Angle& other)
        {
            return operator+(-other);
        }

        [[nodiscard]]
        constexpr Angle operator-() const
        {
            return Angle{-m_angle};
        }
    };
} // namespace omath
