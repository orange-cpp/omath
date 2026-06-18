//
// Created by Vlad on 02.09.2024.
//

#pragma once
#include "omath/internal/constexpr_math.hpp"
#include <cmath>
#include <format>
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
        using ContainedType = Type;
        Type x = static_cast<Type>(0);
        Type y = static_cast<Type>(0);

        // Constructors
        constexpr Vector2() = default;

        template<class CastedType>
        requires std::is_arithmetic_v<CastedType>
        [[nodiscard("You must use casted vector")]] constexpr explicit operator Vector2<CastedType>() const noexcept
        {
            return {static_cast<CastedType>(x), static_cast<CastedType>(y)};
        }
        constexpr Vector2(const Type& x, const Type& y) noexcept: x(x), y(y)
        {
        }

        // Equality operators
        [[nodiscard("You must use comparison result")]]
        constexpr bool operator==(const Vector2& other) const noexcept
        {
            return x == other.x && y == other.y;
        }

        [[nodiscard("You must use comparison result")]]
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
        [[nodiscard("You must use distance")]]
        constexpr Type distance_to(const Vector2& other) const noexcept
        {
            return internal::sqrt(distance_to_sqr(other));
        }

        [[nodiscard("You must use squared distance")]]
        constexpr Type distance_to_sqr(const Vector2& other) const noexcept
        {
            return (x - other.x) * (x - other.x) + (y - other.y) * (y - other.y);
        }

        [[nodiscard("You must use dot product")]]
        constexpr Type dot(const Vector2& other) const noexcept
        {
            return x * other.x + y * other.y;
        }

#ifndef _MSC_VER
        [[nodiscard("You must use length")]] constexpr Type length() const noexcept
        {
            return internal::hypot(this->x, this->y);
        }

        [[nodiscard("You must use normalized vector")]] constexpr Vector2 normalized() const noexcept
        {
            const Type len = length();
            return len > 0.f ? *this / len : *this;
        }
#else
        [[nodiscard("You must use length")]]
        constexpr Type length() const noexcept
        {
            return internal::hypot(x, y);
        }

        [[nodiscard("You must use normalized vector")]]
        constexpr Vector2 normalized() const noexcept
        {
            const Type len = length();
            return len > static_cast<Type>(0) ? *this / len : *this;
        }
#endif
        [[nodiscard("You must use squared length")]]
        constexpr Type length_sqr() const noexcept
        {
            return x * x + y * y;
        }

        constexpr Vector2& abs() noexcept
        {
            // FIXME: Replace with std::abs, if it will become constexprable
            x = x < static_cast<Type>(0) ? -x : x;
            y = y < static_cast<Type>(0) ? -y : y;
            return *this;
        }
        [[nodiscard("You must use absed vector")]]
        constexpr Vector2 abs() const noexcept
        {
            return Vector2{*this}.abs();
        }

        [[nodiscard("You must use negated vector")]]
        constexpr Vector2 operator-() const noexcept
        {
            return {-x, -y};
        }

        // Binary arithmetic operators
        [[nodiscard("You must use result vector")]]
        constexpr Vector2 operator+(const Vector2& other) const noexcept
        {
            return {x + other.x, y + other.y};
        }

        [[nodiscard("You must use result vector")]]
        constexpr Vector2 operator-(const Vector2& other) const noexcept
        {
            return {x - other.x, y - other.y};
        }

        [[nodiscard("You must use result vector")]]
        constexpr Vector2 operator*(const Type& value) const noexcept
        {
            return {x * value, y * value};
        }

        [[nodiscard("You must use result vector")]]
        constexpr Vector2 operator/(const Type& value) const noexcept
        {
            return {x / value, y / value};
        }

        // Sum of elements
        [[nodiscard("You must use sum of elements")]]
        constexpr Type sum() const noexcept
        {
            return x + y;
        }

        [[nodiscard("You must use comparison result")]]
        constexpr bool operator<(const Vector2& other) const noexcept
        {
            return length() < other.length();
        }
        [[nodiscard("You must use comparison result")]]
        constexpr bool operator>(const Vector2& other) const noexcept
        {
            return length() > other.length();
        }

        [[nodiscard("You must use comparison result")]]
        constexpr bool operator<=(const Vector2& other) const noexcept
        {
            return length() <= other.length();
        }

        [[nodiscard("You must use comparison result")]]
        constexpr bool operator>=(const Vector2& other) const noexcept
        {
            return length() >= other.length();
        }

        [[nodiscard("You must use tuple")]]
        constexpr std::tuple<Type, Type> as_tuple() const noexcept
        {
            return std::make_tuple(x, y);
        }

        [[nodiscard("You must use array")]]
        constexpr std::array<Type, 2> as_array() const noexcept
        {
            return {x, y};
        }
#ifdef OMATH_IMGUI_INTEGRATION
        [[nodiscard("You must use ImVec2")]]
        constexpr ImVec2 to_im_vec2() const noexcept
        {
            return {static_cast<float>(this->x), static_cast<float>(this->y)};
        }
        [[nodiscard("You must use vector from ImVec2")]]
        static Vector2 from_im_vec2(const ImVec2& other) noexcept
        {
            return {static_cast<Type>(other.x), static_cast<Type>(other.y)};
        }
#endif
    };
} // namespace omath

template<class Type>
struct std::hash<omath::Vector2<Type>> // NOLINT(*-dcl58-cpp)
{
    [[nodiscard("You must use hash value")]]
    std::size_t operator()(const omath::Vector2<Type>& vec) const noexcept
    {
        std::size_t hash = 0;
        constexpr std::hash<Type> hasher;

        hash ^= hasher(vec.x) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= hasher(vec.y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

        return hash;
    }
};

template<class Type>
struct std::formatter<omath::Vector2<Type>> // NOLINT(*-dcl58-cpp)
{
    [[nodiscard("You must use parse iterator")]]
    static constexpr auto parse(std::format_parse_context& ctx) noexcept
    {
        return ctx.begin();
    }

    template<class FormatContext>
    [[nodiscard("You must use format iterator")]]
    static auto format(const omath::Vector2<Type>& vec, FormatContext& ctx)
    {
        if constexpr (std::is_same_v<typename FormatContext::char_type, char>)
            return std::format_to(ctx.out(), "[{}, {}]", vec.x, vec.y);

        if constexpr (std::is_same_v<typename FormatContext::char_type, wchar_t>)
            return std::format_to(ctx.out(), L"[{}, {}]", vec.x, vec.y);

        if constexpr (std::is_same_v<typename FormatContext::char_type, char8_t>)
            return std::format_to(ctx.out(), u8"[{}, {}]", vec.x, vec.y);
    }
};
