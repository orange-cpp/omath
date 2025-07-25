//
// Created by vlad on 10/28/23.
//

#pragma once

#include "omath/angle.hpp"
#include "omath/vector2.hpp"
#include <cstdint>
#include <expected>
#include <functional>

namespace omath
{

    enum class Vector3Error
    {
        IMPOSSIBLE_BETWEEN_ANGLE,
    };

    template<class Type>
    requires std::is_arithmetic_v<Type>
    class Vector3 : public Vector2<Type>
    {
    public:
        Type z = static_cast<Type>(0);
        constexpr Vector3(const Type& x, const Type& y, const Type& z) noexcept: Vector2<Type>(x, y), z(z)
        {
        }
        constexpr Vector3() noexcept: Vector2<Type>() {};

        [[nodiscard]] constexpr bool operator==(const Vector3& other) const noexcept
        {
            return Vector2<Type>::operator==(other) && (other.z == z);
        }

        [[nodiscard]] constexpr bool operator!=(const Vector3& other) const noexcept
        {
            return !(*this == other);
        }

        constexpr Vector3& operator+=(const Vector3& other) noexcept
        {
            Vector2<Type>::operator+=(other);
            z += other.z;

            return *this;
        }

        constexpr Vector3& operator-=(const Vector3& other) noexcept
        {
            Vector2<Type>::operator-=(other);
            z -= other.z;

            return *this;
        }

        constexpr Vector3& operator*=(const Type& value) noexcept
        {
            Vector2<Type>::operator*=(value);
            z *= value;

            return *this;
        }

        constexpr Vector3& operator*=(const Vector3& other) noexcept
        {
            Vector2<Type>::operator*=(other);
            z *= other.z;

            return *this;
        }

        constexpr Vector3& operator/=(const Vector3& other) noexcept
        {
            Vector2<Type>::operator/=(other);
            z /= other.z;

            return *this;
        }

        constexpr Vector3& operator+=(const Type& value) noexcept
        {
            Vector2<Type>::operator+=(value);
            z += value;

            return *this;
        }

        constexpr Vector3& operator/=(const Type& value) noexcept
        {
            Vector2<Type>::operator/=(value);
            z /= value;

            return *this;
        }

        constexpr Vector3& operator-=(const Type& value) noexcept
        {
            Vector2<Type>::operator-=(value);
            z -= value;

            return *this;
        }

        constexpr Vector3& abs() noexcept
        {
            Vector2<Type>::abs();
            z = z < 0.f ? -z : z;

            return *this;
        }

        [[nodiscard]] constexpr Type distance_to_sqr(const Vector3& other) const noexcept
        {
            return (*this - other).length_sqr();
        }

        [[nodiscard]] constexpr Type dot(const Vector3& other) const noexcept
        {
            return Vector2<Type>::dot(other) + z * other.z;
        }

#ifndef _MSC_VER
        [[nodiscard]] constexpr Type length() const
        {
            return std::hypot(this->x, this->y, z);
        }

        [[nodiscard]] constexpr Type length_2d() const
        {
            return Vector2<Type>::length();
        }
        [[nodiscard]] Type distance_to(const Vector3& other) const
        {
            return (*this - other).length();
        }
        [[nodiscard]] constexpr Vector3 normalized() const
        {
            const Type length_value = this->length();

            return length_value != 0 ? *this / length_value : *this;
        }
#else
        [[nodiscard]] Type length() const noexcept
        {
            return std::hypot(this->x, this->y, z);
        }

        [[nodiscard]] Vector3 normalized() const noexcept
        {
            const Type len = this->length();

            return len != 0 ? *this / len : *this;
        }

        [[nodiscard]] Type length_2d() const noexcept
        {
            return Vector2<Type>::length();
        }

        [[nodiscard]] Type distance_to(const Vector3& vOther) const noexcept
        {
            return (*this - vOther).length();
        }
#endif

        [[nodiscard]] constexpr Type length_sqr() const noexcept
        {
            return Vector2<Type>::length_sqr() + z * z;
        }

        [[nodiscard]] constexpr Vector3 operator-() const noexcept
        {
            return {-this->x, -this->y, -z};
        }

        [[nodiscard]] constexpr Vector3 operator+(const Vector3& other) const noexcept
        {
            return {this->x + other.x, this->y + other.y, z + other.z};
        }

        [[nodiscard]] constexpr Vector3 operator-(const Vector3& other) const noexcept
        {
            return {this->x - other.x, this->y - other.y, z - other.z};
        }

        [[nodiscard]] constexpr Vector3 operator*(const Type& value) const noexcept
        {
            return {this->x * value, this->y * value, z * value};
        }

        [[nodiscard]] constexpr Vector3 operator*(const Vector3& other) const noexcept
        {
            return {this->x * other.x, this->y * other.y, z * other.z};
        }

        [[nodiscard]] constexpr Vector3 operator/(const Type& value) const noexcept
        {
            return {this->x / value, this->y / value, z / value};
        }

        [[nodiscard]] constexpr Vector3 operator/(const Vector3& other) const noexcept
        {
            return {this->x / other.x, this->y / other.y, z / other.z};
        }

        [[nodiscard]] constexpr Vector3 cross(const Vector3& other) const noexcept
        {
            return {this->y * other.z - z * other.y, z * other.x - this->x * other.z,
                    this->x * other.y - this->y * other.x};
        }

        [[nodiscard]] constexpr Type sum() const noexcept
        {
            return sum_2d() + z;
        }

        [[nodiscard]] std::expected<Angle<float, 0.f, 180.f, AngleFlags::Clamped>, Vector3Error>
        angle_between(const Vector3& other) const noexcept
        {
            const auto bottom = length() * other.length();

            if (bottom == 0.f)
                return std::unexpected(Vector3Error::IMPOSSIBLE_BETWEEN_ANGLE);

            return Angle<float, 0.f, 180.f, AngleFlags::Clamped>::from_radians(std::acos(dot(other) / bottom));
        }

        [[nodiscard]] bool is_perpendicular(const Vector3& other) const noexcept
        {
            if (const auto angle = angle_between(other))
                return angle->as_degrees() == 90.f;

            return false;
        }

        [[nodiscard]] constexpr Type sum_2d() const noexcept
        {
            return Vector2<Type>::sum();
        }

        [[nodiscard]] constexpr std::tuple<Type, Type, Type> as_tuple() const noexcept
        {
            return std::make_tuple(this->x, this->y, z);
        }

        [[nodiscard]] Vector3 view_angle_to(const Vector3& other) const noexcept
        {
            const auto distance = distance_to(other);
            const auto delta = other - *this;

            return {angles::radians_to_degrees(std::asin(delta.z / distance)),
                    angles::radians_to_degrees(std::atan2(delta.y, delta.x)), 0};
        }
    };
} // namespace omath
// ReSharper disable once CppRedundantNamespaceDefinition
namespace std
{
    template<> struct hash<omath::Vector3<float>>
    {
        std::size_t operator()(const omath::Vector3<float>& vec) const noexcept
        {
            std::size_t hash = 0;
            constexpr std::hash<float> hasher;

            hash ^= hasher(vec.x) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= hasher(vec.y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= hasher(vec.z) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

            return hash;
        }
    };
} // namespace std
