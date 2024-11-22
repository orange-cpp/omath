//
// Created by Vlad on 02.09.2024.
//

#pragma once
#include <tuple>
#include <cmath>


namespace omath
{
    class Vector2
    {
    public:
        float x = 0.f;
        float y = 0.f;

        // Constructors
        constexpr Vector2() = default;

        constexpr Vector2(const float x, const float y) : x(x), y(y) {}

        // Equality operators
        [[nodiscard]]
        constexpr bool operator==(const Vector2& src) const
        {
            return x == src.x && y == src.y;
        }

        [[nodiscard]]
        constexpr bool operator!=(const Vector2& src) const
        {
            return !(*this == src);
        }

        // Compound assignment operators
        constexpr Vector2& operator+=(const Vector2& v)
        {
            x += v.x;
            y += v.y;

            return *this;
        }

        constexpr Vector2& operator-=(const Vector2& v)
        {
            x -= v.x;
            y -= v.y;

            return *this;
        }

        constexpr Vector2& operator*=(const Vector2& v)
        {
            x *= v.x;
            y *= v.y;

            return *this;
        }

        constexpr Vector2& operator/=(const Vector2& v)
        {
            x /= v.x;
            y /= v.y;

            return *this;
        }

        constexpr Vector2& operator*=(const float fl)
        {
            x *= fl;
            y *= fl;

            return *this;
        }

        constexpr Vector2& operator/=(const float fl)
        {
            x /= fl;
            y /= fl;

            return *this;
        }

        constexpr Vector2& operator+=(const float fl)
        {
            x += fl;
            y += fl;

            return *this;
        }

        constexpr Vector2& operator-=(const float fl)
        {
            x -= fl;
            y -= fl;

            return *this;
        }

        // Basic vector operations
        [[nodiscard]] float DistTo(const Vector2& vOther) const
        {
            return std::sqrt(DistToSqr(vOther));
        }

        [[nodiscard]] constexpr float DistToSqr(const Vector2& vOther) const
        {
            return (x - vOther.x) * (x - vOther.x) + (y - vOther.y) * (y - vOther.y);
        }

        [[nodiscard]] constexpr float Dot(const Vector2& vOther) const
        {
            return x * vOther.x + y * vOther.y;
        }

#ifndef _MSC_VER
        [[nodiscard]] constexpr float Length() const
        {
            return std::hypot(x, y);
        }

        [[nodiscard]] constexpr Vector2 Normalized() const
        {
            const float len = Length();
            return len > 0.f ? *this / len : *this;
        }
#else
        [[nodiscard]] float Length() const
        {
            return std::hypot(x, y);
        }

        [[nodiscard]] Vector2 Normalized() const
        {
            const float len = Length();
            return len > 0.f ? *this / len : *this;
        }
#endif
        [[nodiscard]] constexpr float LengthSqr() const
        {
            return x * x + y * y;
        }

        constexpr Vector2& Abs()
        {
            //FIXME: Replace with std::abs, if it will become constexprable
            x = x < 0 ? -x : x;
            y = y < 0 ? -y : y;
            return *this;
        }

        template<class type>
        [[nodiscard]] constexpr const type& As() const
        {
            return *reinterpret_cast<const type*>(this);
        }
        template<class type>
        [[nodiscard]] constexpr type& As()
        {
            return *reinterpret_cast<type*>(this);
        }

        [[nodiscard]] constexpr Vector2 operator-() const
        {
            return {-x, -y};
        }

        // Binary arithmetic operators
        [[nodiscard]] constexpr Vector2 operator+(const Vector2& v) const
        {
            return {x + v.x, y + v.y};
        }

        [[nodiscard]] constexpr Vector2 operator-(const Vector2& v) const
        {
            return {x - v.x, y - v.y};
        }

        [[nodiscard]] constexpr Vector2 operator*(const float fl) const
        {
            return {x * fl, y * fl};
        }

        [[nodiscard]] constexpr Vector2 operator/(const float fl) const
        {
            return {x / fl, y / fl};
        }

         // Sum of elements
        [[nodiscard]] constexpr float Sum() const
        {
            return x + y;
        }

        [[nodiscard]]
        constexpr std::tuple<float, float> AsTuple() const
        {
            return std::make_tuple(x, y);
        }
    };
}