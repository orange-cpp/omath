//
// Vector4.h
//
#pragma once

#include <omath/Vector3.hpp>
#include <algorithm>

namespace omath
{
    template <class Type>
    class Vector4 : public Vector3<Type>
    {
    public:
        Type w;

        constexpr Vector4(const Type& x, const Type& y, const Type& z, const Type& w) : Vector3<Type>(x, y, z), w(w) {}
        constexpr Vector4() : Vector3<Type>(), w(0) {};

        [[nodiscard]]
        constexpr bool operator==(const Vector4& src) const
        {
            return Vector3<Type>::operator==(src) && w == src.w;
        }

        [[nodiscard]]
        constexpr bool operator!=(const Vector4& src) const
        {
            return !(*this == src);
        }

        constexpr Vector4& operator+=(const Vector4& v)
        {
            Vector3<Type>::operator+=(v);
            w += v.w;

            return *this;
        }

        constexpr Vector4& operator-=(const Vector4& v)
        {
            Vector3<Type>::operator-=(v);
            w -= v.w;

            return *this;
        }

        constexpr Vector4& operator*=(const float scalar)
        {
            Vector3<Type>::operator*=(scalar);
            w *= scalar;

            return *this;
        }

        constexpr Vector4& operator*=(const Vector4& v)
        {
            Vector3<Type>::operator*=(v);
            w *= v.w;

            return *this;
        }

        constexpr Vector4& operator/=(const float scalar)
        {
            Vector3<Type>::operator/=(scalar);
            w /= scalar;

            return *this;
        }

        constexpr Vector4& operator/=(const Vector4& v)
        {
            Vector3<Type>::operator/=(v);
            w /= v.w;
            return *this;
        }

        [[nodiscard]] constexpr Type LengthSqr() const
        {
            return Vector3<Type>::LengthSqr() + w * w;
        }

        [[nodiscard]] constexpr float Dot(const Vector4& vOther) const
        {
            return Vector3<Type>::Dot(vOther) + w * vOther.w;
        }

        [[nodiscard]] Vector3<Type> Length() const
        {
            return std::sqrt(LengthSqr());
        }

        constexpr Vector4& Abs()
        {
            Vector3<Type>::Abs();
            w = w < 0.f ? -w : w;

            return *this;
        }
        constexpr Vector4& Clamp(const float min, const float max)
        {
            this->x = std::clamp(this->x, min, max);
            this->y = std::clamp(this->y, min, max);
            this->z = std::clamp(this->z, min, max);

            return *this;
        }

        [[nodiscard]]
        constexpr Vector4 operator-() const
        {
            return {-this->x, -this->y, -this->z, -w};
        }

        [[nodiscard]]
        constexpr Vector4 operator+(const Vector4& v) const
        {
            return {this->x + v.x, this->y + v.y, this->z + v.z, w + v.w};
        }

        [[nodiscard]]
        constexpr Vector4 operator-(const Vector4& v) const
        {
            return {this->x - v.x, this->y - v.y, this->z - v.z, w - v.w};
        }

        [[nodiscard]]
        constexpr Vector4 operator*(const float scalar) const
        {
            return {this->x * scalar, this->y * scalar, this->z * scalar, w * scalar};
        }

        [[nodiscard]]
        constexpr Vector4 operator*(const Vector4& v) const
        {
            return {this->x * v.x, this->y * v.y, this->z * v.z, w * v.w};
        }

        [[nodiscard]]
        constexpr Vector4 operator/(const float scalar) const
        {
            return {this->x / scalar, this->y / scalar, this->z / scalar, w / scalar};
        }

        [[nodiscard]]
        constexpr Vector4 operator/(const Vector4& v) const
        {
            return {this->x / v.x, this->y / v.y, this->z / v.z, w / v.w};
        }

        [[nodiscard]]
        constexpr Type Sum() const
        {
            return Vector3<Type>::Sum() + w;
        }
    };
}
