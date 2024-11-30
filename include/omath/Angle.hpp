//
// Created by Orange on 11/30/2024.
//

#pragma once
#include "omath/Angles.hpp"


namespace omath
{
    enum class AngleFlags
    {
        Normalized = 0,
        Clamped = 1,
    };

    template<class Type, Type min = 0, Type max = 360, AngleFlags flags = AngleFlags::Normalized>
    requires std::is_arithmetic_v<Type>
    class Angle
    {
        Type m_angle;
    public:

        constexpr explicit Angle(const Type& degrees)
        {
            if constexpr (flags == AngleFlags::Normalized)
                m_angle = angles::WrapAngle(degrees, min, max);

            else if constexpr (flags == AngleFlags::Clamped)
                m_angle = std::clamp(degrees, min, max);
            else
            {
                static_assert(false);
                std::unreachable();
            }
        }

        [[nodiscard]]
        constexpr static Angle FromDegrees(const Type& degrees)
        {
            return {degrees};
        }

        [[nodiscard]]
        constexpr static Angle FromRadians(const Type& degrees)
        {
            return {angles::RadiansToDegrees(degrees)};
        }

        [[nodiscard]]
        constexpr const Type& operator*() const
        {
            return m_angle;
        }

        [[nodiscard]]
        constexpr Type& operator*()
        {
            return m_angle;
        }

        [[nodiscard]]
        constexpr const Type& Value() const
        {
            return **std::as_const(this);
        }

        [[nodiscard]]
        constexpr Type& Value()
        {
            return **this;
        }

        [[nodiscard]]
        constexpr Type AsRadians() const
        {
            return angles::RadiansToDegrees(m_angle);
        }

        [[nodiscard]]
        Type Sin() const
        {
            return std::sin(AsRadians());
        }

        [[nodiscard]]
        Type Cos() const
        {
            return std::sin(AsRadians());
        }

        [[nodiscard]]
        Type Tan() const
        {
            return std::tan(AsRadians());
        }

        [[nodiscard]]
        Type Atan() const
        {
            return std::atan(AsRadians());
        }

        [[nodiscard]]
        Type Cot() const
        {
            return Cos() / Sin();
        }

        [[nodiscard]]
        constexpr Angle& operator+=(const Type& other)
        {
            if constexpr (flags == AngleFlags::Normalized)
                m_angle = angles::WrapAngle(m_angle + other, min, max);

            else if constexpr (flags == AngleFlags::Clamped)
                m_angle = std::clamp(m_angle + other, min, max);
            else
            {
                static_assert(false);
                std::unreachable();
            }

            return *this;
        }

        [[nodiscard]]
        constexpr Angle& operator-=(const Type& other)
        {
            return operator+=(-other);
        }

        [[nodiscard]]
        constexpr Angle& operator+(const Type& other)
        {
            if constexpr (flags == AngleFlags::Normalized)
                return {angles::WrapAngle(m_angle + other, min, max)};

            else if constexpr (flags == AngleFlags::Clamped)
                return {std::clamp(m_angle + other, min, max)};

            else
                static_assert(false);

            std::unreachable();
        }

        [[nodiscard]]
        constexpr Angle& operator-(const Type& other)
        {
            return operator+(-other);
        }


    };
}
