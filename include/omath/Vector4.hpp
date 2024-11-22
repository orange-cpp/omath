//
// Vector4.h
//
#pragma once

#include <omath/Vector3.hpp>
#include <algorithm>


namespace omath
{
    class Vector4 : public Vector3
    {
    public:
        float w;

        constexpr Vector4(const float x, const float y, const float z, const float w) : Vector3(x, y, z), w(w) {}
        constexpr Vector4() : Vector3(), w(0.f) {};

        [[nodiscard]]
        constexpr bool operator==(const Vector4& src) const
        {
            return Vector3::operator==(src) && w == src.w;
        }

        [[nodiscard]]
        constexpr bool operator!=(const Vector4& src) const
        {
            return !(*this == src);
        }

        constexpr Vector4& operator+=(const Vector4& v)
        {
            Vector3::operator+=(v);
            w += v.w;

            return *this;
        }

        constexpr Vector4& operator-=(const Vector4& v)
        {
            Vector3::operator-=(v);
            w -= v.w;

            return *this;
        }

        constexpr Vector4& operator*=(const float scalar)
        {
            Vector3::operator*=(scalar);
            w *= scalar;

            return *this;
        }

        constexpr Vector4& operator*=(const Vector4& v)
        {
            Vector3::operator*=(v);
            w *= v.w;

            return *this;
        }

        constexpr Vector4& operator/=(const float scalar)
        {
            Vector3::operator/=(scalar);
            w /= scalar;

            return *this;
        }

        constexpr Vector4& operator/=(const Vector4& v)
        {
            Vector3::operator/=(v);
            w /= v.w;
            return *this;
        }

        [[nodiscard]] constexpr float LengthSqr() const
        {
            return Vector3::LengthSqr() + w * w;
        }

        [[nodiscard]] constexpr float Dot(const Vector4& vOther) const
        {
            return Vector3::Dot(vOther) + w * vOther.w;
        }

        [[nodiscard]] float Length() const;

        constexpr Vector4& Abs()
        {
            Vector3::Abs();
            w = w < 0.f ? -w : w;

            return *this;
        }
        constexpr Vector4& Clamp(const float min, const float max)
        {
            x = std::clamp(x, min, max);
            y = std::clamp(y, min, max);
            z = std::clamp(z, min, max);

            return *this;
        }

        [[nodiscard]]
        constexpr Vector4 operator-() const
        {
            return {-x, -y, -z, -w};
        }

        [[nodiscard]]
        constexpr Vector4 operator+(const Vector4& v) const
        {
            return {x + v.x, y + v.y, z + v.z, w + v.w};
        }

        [[nodiscard]]
        constexpr Vector4 operator-(const Vector4& v) const
        {
            return {x - v.x, y - v.y, z - v.z, w - v.w};
        }

        [[nodiscard]]
        constexpr Vector4 operator*(const float scalar) const
        {
            return {x * scalar, y * scalar, z * scalar, w * scalar};
        }

        [[nodiscard]]
        constexpr Vector4 operator*(const Vector4& v) const
        {
            return {x * v.x, y * v.y, z * v.z, w * v.w};
        }

        [[nodiscard]]
        constexpr Vector4 operator/(const float scalar) const
        {
            return {x / scalar, y / scalar, z / scalar, w / scalar};
        }

        [[nodiscard]]
        constexpr Vector4 operator/(const Vector4& v) const
        {
            return {x / v.x, y / v.y, z / v.z, w / v.w};
        }

        [[nodiscard]]
        constexpr float Sum() const
        {
            return Vector3::Sum() + w;
        }
    };
}
