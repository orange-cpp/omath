//
// Vector4.h
//
#pragma once

#include <algorithm>
#include <omath/vector3.hpp>

namespace omath
{
    template<class Type>
    class Vector4 : public Vector3<Type>
    {
    public:
        Type w;

        constexpr Vector4(const Type& x, const Type& y, const Type& z, const Type& w): Vector3<Type>(x, y, z), w(w)
        {
        }
        constexpr Vector4(): Vector3<Type>(), w(0) {};

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

        [[nodiscard]] constexpr Type length_sqr() const
        {
            return Vector3<Type>::length_sqr() + w * w;
        }

        [[nodiscard]] constexpr Type dot(const Vector4& other) const
        {
            return Vector3<Type>::dot(other) + w * other.w;
        }

        [[nodiscard]] Vector3<Type> length() const
        {
            return std::sqrt(length_sqr());
        }

        constexpr Vector4& abs()
        {
            Vector3<Type>::abs();
            w = w < 0.f ? -w : w;

            return *this;
        }
        constexpr Vector4& clamp(const Type& min, const Type& max)
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
        constexpr Vector4 operator*(const Type& scalar) const
        {
            return {this->x * scalar, this->y * scalar, this->z * scalar, w * scalar};
        }

        [[nodiscard]]
        constexpr Vector4 operator*(const Vector4& v) const
        {
            return {this->x * v.x, this->y * v.y, this->z * v.z, w * v.w};
        }

        [[nodiscard]]
        constexpr Vector4 operator/(const Type& scalar) const
        {
            return {this->x / scalar, this->y / scalar, this->z / scalar, w / scalar};
        }

        [[nodiscard]]
        constexpr Vector4 operator/(const Vector4& v) const
        {
            return {this->x / v.x, this->y / v.y, this->z / v.z, w / v.w};
        }

        [[nodiscard]]
        constexpr Type sum() const
        {
            return Vector3<Type>::sum() + w;
        }

#ifdef OMATH_IMGUI_INTEGRATION
        [[nodiscard]]
        ImVec4 ToImVec4() const
        {
            return {
                    static_cast<float>(this->x),
                    static_cast<float>(this->y),
                    static_cast<float>(this->z),
                    static_cast<float>(w),
            };
        }
#endif
    };
} // namespace omath
