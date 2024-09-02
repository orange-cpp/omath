//
// Created by Vlad on 02.09.2024.
//
#include "omath/Vector2.h"
#include <cmath>


namespace omath
{
    // Constructors
    Vector2::Vector2(const float x, const float y) : x(x), y(y) {}

    // Equality operators
    bool Vector2::operator==(const Vector2& src) const
    {
        return x == src.x && y == src.y;
    }

    bool Vector2::operator!=(const Vector2& src) const
    {
        return !(*this == src);
    }

    // Compound assignment operators
    Vector2& Vector2::operator+=(const Vector2& v)
    {
        x += v.x;
        y += v.y;
        return *this;
    }

    Vector2& Vector2::operator-=(const Vector2& v)
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    Vector2& Vector2::operator*=(float fl)
    {
        x *= fl;
        y *= fl;
        return *this;
    }

    Vector2& Vector2::operator/=(float fl)
    {
        x /= fl;
        y /= fl;
        return *this;
    }

    Vector2& Vector2::operator+=(float fl)
    {
        x += fl;
        y += fl;
        return *this;
    }

    Vector2& Vector2::operator-=(float fl)
            {
        x -= fl;
        y -= fl;
        return *this;
    }

    // Basic vector operations
    float Vector2::DistTo(const Vector2& vOther) const
    {
        return std::sqrt(DistToSqr(vOther));
    }

    float Vector2::DistToSqr(const Vector2& vOther) const
    {
        return (x - vOther.x) * (x - vOther.x) + (y - vOther.y) * (y - vOther.y);
    }

    float Vector2::Dot(const Vector2& vOther) const
    {
        return x * vOther.x + y * vOther.y;
    }

    float Vector2::Length() const
    {
        return std::sqrt(x * x + y * y);
    }

    float Vector2::LengthSqr() const
    {
        return x * x + y * y;
    }

    Vector2& Vector2::Abs()
    {
        x = std::abs(x);
        y = std::abs(y);
        return *this;
    }

    // Unary negation operator
    Vector2 Vector2::operator-() const
    {
        return {-x, -y};
    }

    // Binary arithmetic operators
    Vector2 Vector2::operator+(const Vector2& v) const
    {
        return {x + v.x, y + v.y};
    }

    Vector2 Vector2::operator-(const Vector2& v) const
    {
        return {x - v.x, y - v.y};
    }

    Vector2 Vector2::operator*(float fl) const
    {
        return {x * fl, y * fl};
    }

    Vector2 Vector2::operator/(float fl) const
    {
        return {x / fl, y / fl};
    }

    // Normalize the vector
    Vector2 Vector2::Normalized() const
    {
        float len = Length();
        if (len > 0.f) {
            return {x / len, y / len};
        }
        return {0.f, 0.f};
    }

    // Sum of elements
    float Vector2::Sum() const
    {
        return x + y;
    }

    Vector2 &Vector2::operator*=(const Vector2 &v)
    {
        x *= v.x;
        y *= v.y;

        return *this;
    }

    Vector2 &Vector2::operator/=(const Vector2 &v)
    {
        x /= v.x;
        y /= v.y;

        return *this;
    }

    std::tuple<float, float> Vector2::AsTuple() const
    {
        return std::make_tuple(x, y);
    }
}