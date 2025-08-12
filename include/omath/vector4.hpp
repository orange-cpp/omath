//
// Vector4.h
//
#pragma once

#include <algorithm>
#include <omath/vector3.hpp>

namespace omath
{
    template<class Type>
    requires std::is_arithmetic_v<Type>
    class Vector4 : public Vector3<Type>
    {
    public:
        Type w;

        constexpr Vector4(const Type& x, const Type& y, const Type& z, const Type& w): Vector3<Type>(x, y, z), w(w)
        {
        }
        constexpr Vector4() noexcept: Vector3<Type>(), w(static_cast<Type>(0)) {};

        [[nodiscard]]
        constexpr bool operator==(const Vector4& other) const noexcept
        {
            return Vector3<Type>::operator==(other) && w == other.w;
        }

        [[nodiscard]]
        constexpr bool operator!=(const Vector4& other) const noexcept
        {
            return !(*this == other);
        }

        constexpr Vector4& operator+=(const Vector4& other) noexcept
        {
            Vector3<Type>::operator+=(other);
            w += other.w;

            return *this;
        }

        constexpr Vector4& operator-=(const Vector4& other) noexcept
        {
            Vector3<Type>::operator-=(other);
            w -= other.w;

            return *this;
        }

        constexpr Vector4& operator*=(const Type& value) noexcept
        {
            Vector3<Type>::operator*=(value);
            w *= value;

            return *this;
        }

        constexpr Vector4& operator*=(const Vector4& other) noexcept
        {
            Vector3<Type>::operator*=(other);
            w *= other.w;

            return *this;
        }

        constexpr Vector4& operator/=(const Type& value) noexcept
        {
            Vector3<Type>::operator/=(value);
            w /= value;

            return *this;
        }

        constexpr Vector4& operator/=(const Vector4& other) noexcept
        {
            Vector3<Type>::operator/=(other);
            w /= other.w;
            return *this;
        }

        [[nodiscard]] constexpr Type length_sqr() const noexcept
        {
            return Vector3<Type>::length_sqr() + w * w;
        }

        [[nodiscard]] constexpr Type dot(const Vector4& other) const noexcept
        {
            return Vector3<Type>::dot(other) + w * other.w;
        }

        [[nodiscard]] Type length() const noexcept
        {
            return std::sqrt(length_sqr());
        }

        constexpr Vector4& abs() noexcept
        {
            Vector3<Type>::abs();
            w = w < 0.f ? -w : w;

            return *this;
        }
        constexpr Vector4& clamp(const Type& min, const Type& max) noexcept
        {
            this->x = std::clamp(this->x, min, max);
            this->y = std::clamp(this->y, min, max);
            this->z = std::clamp(this->z, min, max);

            return *this;
        }

        [[nodiscard]]
        constexpr Vector4 operator-() const noexcept
        {
            return {-this->x, -this->y, -this->z, -w};
        }

        [[nodiscard]]
        constexpr Vector4 operator+(const Vector4& other) const noexcept
        {
            return {this->x + other.x, this->y + other.y, this->z + other.z, w + other.w};
        }

        [[nodiscard]]
        constexpr Vector4 operator-(const Vector4& other) const noexcept
        {
            return {this->x - other.x, this->y - other.y, this->z - other.z, w - other.w};
        }

        [[nodiscard]]
        constexpr Vector4 operator*(const Type& value) const noexcept
        {
            return {this->x * value, this->y * value, this->z * value, w * value};
        }

        [[nodiscard]]
        constexpr Vector4 operator*(const Vector4& other) const noexcept
        {
            return {this->x * other.x, this->y * other.y, this->z * other.z, w * other.w};
        }

        [[nodiscard]]
        constexpr Vector4 operator/(const Type& value) const noexcept
        {
            return {this->x / value, this->y / value, this->z / value, w / value};
        }

        [[nodiscard]]
        constexpr Vector4 operator/(const Vector4& other) const noexcept
        {
            return {this->x / other.x, this->y / other.y, this->z / other.z, w / other.w};
        }

        [[nodiscard]]
        constexpr Type sum() const noexcept
        {
            return Vector3<Type>::sum() + w;
        }

        [[nodiscard]]
        bool operator<(const Vector4& other) const noexcept
        {
            return length() < other.length();
        }

        [[nodiscard]]
        bool operator>(const Vector4& other) const noexcept
        {
            return length() > other.length();
        }

        [[nodiscard]]
        bool operator<=(const Vector4& other) const noexcept
        {
            return length() <= other.length();
        }

        [[nodiscard]]
        bool operator>=(const Vector4& other) const noexcept
        {
            return length() >= other.length();
        }

#ifdef OMATH_IMGUI_INTEGRATION
        [[nodiscard]]
        ImVec4 to_im_vec4() const noexcept
        {
            return {
                    static_cast<float>(this->x),
                    static_cast<float>(this->y),
                    static_cast<float>(this->z),
                    static_cast<float>(w),
            };
        }
        [[nodiscard]]
        static Vector4<float> from_im_vec4(const ImVec4& other) noexcept
        {
            return {static_cast<Type>(other.x), static_cast<Type>(other.y), static_cast<Type>(other.z)};
        }
    }
#endif
};
} // namespace omath
