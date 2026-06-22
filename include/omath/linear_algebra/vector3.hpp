//
// Created by vlad on 10/28/23.
//

#pragma once

#include "omath/internal/constexpr_math.hpp"
#include "omath/linear_algebra/vector2.hpp"
#include "omath/trigonometry/angle.hpp"
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
        using ContainedType = Type;
        Type z = static_cast<Type>(0);
        constexpr Vector3(const Type& x, const Type& y, const Type& z) noexcept: Vector2<Type>(x, y), z(z)
        {
        }
        constexpr Vector3() noexcept: Vector2<Type>() {};

        template<class CastedType>
        requires std::is_arithmetic_v<CastedType>
        [[nodiscard("You must use casted vector")]]
        constexpr explicit operator Vector3<CastedType>() const noexcept
        {
            return {static_cast<CastedType>(this->x), static_cast<CastedType>(this->y),
                    static_cast<CastedType>(this->z)};
        }
        [[nodiscard("You must use comparison result")]]
        constexpr bool operator==(const Vector3& other) const noexcept
        {
            return Vector2<Type>::operator==(other) && (other.z == z);
        }

        [[nodiscard("You must use comparison result")]]
        constexpr bool operator!=(const Vector3& other) const noexcept
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
        [[nodiscard("You must use absed vector")]]
        constexpr Vector3 abs() const noexcept
        {
            return Vector3{*this}.abs();
        }

        [[nodiscard("You must use squared distance")]]
        constexpr Type distance_to_sqr(const Vector3& other) const noexcept
        {
            return (*this - other).length_sqr();
        }

        [[nodiscard("You must use dot product")]]
        constexpr Type dot(const Vector3& other) const noexcept
        {
            return Vector2<Type>::dot(other) + z * other.z;
        }

#ifndef _MSC_VER
        [[nodiscard("You must use length")]] constexpr Type length() const noexcept
        {
            return internal::hypot(this->x, this->y, z);
        }

        [[nodiscard("You must use 2D length")]] constexpr Type length_2d() const noexcept
        {
            return Vector2<Type>::length();
        }
        [[nodiscard("You must use distance")]] constexpr Type distance_to(const Vector3& other) const noexcept
        {
            return (*this - other).length();
        }
        [[nodiscard("You must use normalized vector")]] constexpr Vector3 normalized() const noexcept
        {
            const Type length_value = this->length();

            return length_value != 0 ? *this / length_value : *this;
        }
#else
        [[nodiscard("You must use length")]]
        constexpr Type length() const noexcept
        {
            return internal::hypot(this->x, this->y, this->z);
        }

        [[nodiscard("You must use normalized vector")]]
        constexpr Vector3 normalized() const noexcept
        {
            const Type len = this->length();

            return len != static_cast<Type>(0) ? *this / len : *this;
        }

        [[nodiscard("You must use 2D length")]]
        constexpr Type length_2d() const noexcept
        {
            return Vector2<Type>::length();
        }

        [[nodiscard("You must use distance")]]
        constexpr Type distance_to(const Vector3& v_other) const noexcept
        {
            return (*this - v_other).length();
        }
#endif

        [[nodiscard("You must use squared length")]]
        constexpr Type length_sqr() const noexcept
        {
            return Vector2<Type>::length_sqr() + z * z;
        }

        [[nodiscard("You must use negated vector")]]
        constexpr Vector3 operator-() const noexcept
        {
            return {-this->x, -this->y, -z};
        }

        [[nodiscard("You must use result vector")]]
        constexpr Vector3 operator+(const Vector3& other) const noexcept
        {
            return {this->x + other.x, this->y + other.y, z + other.z};
        }

        [[nodiscard("You must use result vector")]]
        constexpr Vector3 operator-(const Vector3& other) const noexcept
        {
            return {this->x - other.x, this->y - other.y, z - other.z};
        }

        [[nodiscard("You must use result vector")]]
        constexpr Vector3 operator*(const Type& value) const noexcept
        {
            return {this->x * value, this->y * value, z * value};
        }

        [[nodiscard("You must use result vector")]]
        constexpr Vector3 operator*(const Vector3& other) const noexcept
        {
            return {this->x * other.x, this->y * other.y, z * other.z};
        }

        [[nodiscard("You must use result vector")]]
        constexpr Vector3 operator/(const Type& value) const noexcept
        {
            return {this->x / value, this->y / value, z / value};
        }

        [[nodiscard("You must use result vector")]]
        constexpr Vector3 operator/(const Vector3& other) const noexcept
        {
            return {this->x / other.x, this->y / other.y, z / other.z};
        }

        [[nodiscard("You must use cross product")]]
        constexpr Vector3 cross(const Vector3& other) const noexcept
        {
            return {this->y * other.z - z * other.y, z * other.x - this->x * other.z,
                    this->x * other.y - this->y * other.x};
        }

        [[nodiscard("You must use sum of elements")]]
        constexpr Type sum() const noexcept
        {
            return sum_2d() + z;
        }

        [[nodiscard("You must use direction check result")]]
        constexpr bool point_to_same_direction(const Vector3& other) const
        {
            return dot(other) > static_cast<Type>(0);
        }
        [[nodiscard("You must use angle between vectors")]]
        constexpr std::expected<Angle<float, 0.f, 180.f, AngleFlags::Clamped>, Vector3Error>
        angle_between(const Vector3& other) const noexcept
        {
            const auto bottom = length() * other.length();

            if (bottom == static_cast<Type>(0))
                return std::unexpected(Vector3Error::IMPOSSIBLE_BETWEEN_ANGLE);
            return Angle<float, 0.f, 180.f, AngleFlags::Clamped>::from_radians(internal::acos(dot(other) / bottom));
        }

        [[nodiscard("You must use perpendicularity check result")]]
        constexpr bool is_perpendicular(const Vector3& other, Type epsilon = static_cast<Type>(0.0001)) const noexcept
        {
            if (const auto angle = angle_between(other))
                return std::abs(angle->as_degrees() - static_cast<Type>(90)) <= epsilon;

            return false;
        }

        [[nodiscard("You must use 2D sum")]]
        constexpr Type sum_2d() const noexcept
        {
            return Vector2<Type>::sum();
        }

        [[nodiscard("You must use tuple")]]
        constexpr std::tuple<Type, Type, Type> as_tuple() const noexcept
        {
            return std::make_tuple(this->x, this->y, z);
        }

        [[nodiscard("You must use comparison result")]]
        constexpr bool operator<(const Vector3& other) const noexcept
        {
            return length() < other.length();
        }

        [[nodiscard("You must use comparison result")]]
        constexpr bool operator>(const Vector3& other) const noexcept
        {
            return length() > other.length();
        }

        [[nodiscard("You must use comparison result")]]
        constexpr bool operator<=(const Vector3& other) const noexcept
        {
            return length() <= other.length();
        }

        [[nodiscard("You must use comparison result")]]
        constexpr bool operator>=(const Vector3& other) const noexcept
        {
            return length() >= other.length();
        }

        [[nodiscard("You must use array")]]
        constexpr std::array<Type, 3> as_array() const noexcept
        {
            return {this->x, this->y, z};
        }
    };
} // namespace omath

template<class Type>
struct std::hash<omath::Vector3<Type>> final // NOLINT(*-dcl58-cpp)
{
    // NOTE: Cannot be constexpr because of MSVC
    [[nodiscard("You must use hash value")]]
    std::size_t operator()(const omath::Vector3<Type>& vec) const noexcept
    {
        std::size_t hash = 0;
        constexpr std::hash<Type> hasher;

        hash ^= hasher(vec.x) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= hasher(vec.y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= hasher(vec.z) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

        return hash;
    }
};

template<class Type>
struct std::formatter<omath::Vector3<Type>> final // NOLINT(*-dcl58-cpp)
{
    [[nodiscard("You must use parse iterator")]]
    static constexpr auto parse(std::format_parse_context& ctx) noexcept
    {
        return ctx.begin();
    }

    template<class FormatContext>
    [[nodiscard("You must use format iterator")]]
    static auto format(const omath::Vector3<Type>& vec, FormatContext& ctx)
    {
        if constexpr (std::is_same_v<typename FormatContext::char_type, char>)
            return std::format_to(ctx.out(), "[{}, {}, {}]", vec.x, vec.y, vec.z);

        if constexpr (std::is_same_v<typename FormatContext::char_type, wchar_t>)
            return std::format_to(ctx.out(), L"[{}, {}, {}]", vec.x, vec.y, vec.z);

        if constexpr (std::is_same_v<typename FormatContext::char_type, char8_t>)
            return std::format_to(ctx.out(), u8"[{}, {}, {}]", vec.x, vec.y, vec.z);
    }
};
