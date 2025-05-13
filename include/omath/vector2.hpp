//
// Created by Vlad on 02.09.2024.
//

#pragma once
#include <cmath>
#include <tuple>

#ifdef OMATH_IMGUI_INTEGRATION
#include <imgui.h>
#endif

namespace omath
{

    template<class Type>
    requires std::is_arithmetic_v<Type>
    class Vector2
    {
    public:
        Type x = static_cast<Type>(0);
        Type y = static_cast<Type>(0);

        // Constructors
        constexpr Vector2() = default;

        constexpr Vector2(const Type& x, const Type& y) noexcept: x(x), y(y)
        {
        }

        // Equality operators
        [[nodiscard]]
        constexpr bool operator==(const Vector2& other) const noexcept
        {
            return x == other.x && y == other.y;
        }

        [[nodiscard]]
        constexpr bool operator!=(const Vector2& other) const noexcept
        {
            return !(*this == other);
        }

        // Compound assignment operators
        constexpr Vector2& operator+=(const Vector2& other) noexcept
        {
            x += other.x;
            y += other.y;

            return *this;
        }

        constexpr Vector2& operator-=(const Vector2& other) noexcept
        {
            x -= other.x;
            y -= other.y;

            return *this;
        }

        constexpr Vector2& operator*=(const Vector2& other) noexcept
        {
            x *= other.x;
            y *= other.y;

            return *this;
        }

        constexpr Vector2& operator/=(const Vector2& other) noexcept
        {
            x /= other.x;
            y /= other.y;

            return *this;
        }

        constexpr Vector2& operator*=(const Type& value) noexcept
        {
            x *= value;
            y *= value;

            return *this;
        }

        constexpr Vector2& operator/=(const Type& value) noexcept
        {
            x /= value;
            y /= value;

            return *this;
        }

        constexpr Vector2& operator+=(const Type& value) noexcept
        {
            x += value;
            y += value;

            return *this;
        }

        constexpr Vector2& operator-=(const Type& value) noexcept
        {
            x -= value;
            y -= value;

            return *this;
        }

        // Basic vector operations
        [[nodiscard]] Type distance_to(const Vector2& other) const noexcept
        {
            return std::sqrt(distance_to_sqr(other));
        }

        [[nodiscard]] constexpr Type distance_to_sqr(const Vector2& other) const noexcept
        {
            return (x - other.x) * (x - other.x) + (y - other.y) * (y - other.y);
        }

        [[nodiscard]] constexpr Type dot(const Vector2& other) const noexcept
        {
            return x * other.x + y * other.y;
        }

#ifndef _MSC_VER
        [[nodiscard]] constexpr Type length() const noexcept
        {
            return std::hypot(this->x, this->y);
        }

        [[nodiscard]] constexpr Vector2 normalized() const noexcept
        {
            const Type len = length();
            return len > 0.f ? *this / len : *this;
        }
#else
        [[nodiscard]] Type length() const noexcept
        {
            return std::hypot(x, y);
        }

        [[nodiscard]] Vector2 normalized() const noexcept
        {
            const Type len = length();
            return len > 0.f ? *this / len : *this;
        }
#endif
        [[nodiscard]] constexpr Type length_sqr() const noexcept
        {
            return x * x + y * y;
        }

        constexpr Vector2& abs() noexcept
        {
            // FIXME: Replace with std::abs, if it will become constexprable
            x = x < 0 ? -x : x;
            y = y < 0 ? -y : y;
            return *this;
        }

        [[nodiscard]] constexpr Vector2 operator-() const noexcept
        {
            return {-x, -y};
        }

        // Binary arithmetic operators
        [[nodiscard]] constexpr Vector2 operator+(const Vector2& other) const noexcept
        {
            return {x + other.x, y + other.y};
        }

        [[nodiscard]] constexpr Vector2 operator-(const Vector2& other) const noexcept
        {
            return {x - other.x, y - other.y};
        }

        [[nodiscard]] constexpr Vector2 operator*(const Type& value) const noexcept
        {
            return {x * value, y * value};
        }

        [[nodiscard]] constexpr Vector2 operator/(const Type& value) const noexcept
        {
            return {x / value, y / value};
        }

        // Sum of elements
        [[nodiscard]] constexpr Type sum() const noexcept
        {
            return x + y;
        }

        [[nodiscard]]
        constexpr std::tuple<Type, Type> as_tuple() const noexcept
        {
            return std::make_tuple(x, y);
        }

#ifdef OMATH_IMGUI_INTEGRATION
        [[nodiscard]]
        ImVec2 to_im_vec2() const noexcept
        {
            return {static_cast<float>(this->x), static_cast<float>(this->y)};
        }
#endif
    };
} // namespace omath
