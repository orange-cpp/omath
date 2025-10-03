//
// Created by Vlad on 10/3/2025.
//
module;
#include <cstdint>
#include <expected>
#include <functional>
#include <format>

export module omath.linear_algebra.vector3;

import omath.linear_algebra.vector2;
import omath.trigonometry.angle;

export namespace omath
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

            return len != static_cast<Type>(0) ? *this / len : *this;
        }

        [[nodiscard]] Type length_2d() const noexcept
        {
            return Vector2<Type>::length();
        }

        [[nodiscard]] Type distance_to(const Vector3& v_other) const noexcept
        {
            return (*this - v_other).length();
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

            if (bottom == static_cast<Type>(0))
                return std::unexpected(Vector3Error::IMPOSSIBLE_BETWEEN_ANGLE);

            return Angle<float, 0.f, 180.f, AngleFlags::Clamped>::from_radians(std::acos(dot(other) / bottom));
        }

        [[nodiscard]] bool is_perpendicular(const Vector3& other) const noexcept
        {
            if (const auto angle = angle_between(other))
                return angle->as_degrees() == static_cast<Type>(90);

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

        [[nodiscard]]
        bool operator<(const Vector3& other) const noexcept
        {
            return length() < other.length();
        }

        [[nodiscard]]
        bool operator>(const Vector3& other) const noexcept
        {
            return length() > other.length();
        }

        [[nodiscard]]
        bool operator<=(const Vector3& other) const noexcept
        {
            return length() <= other.length();
        }

        [[nodiscard]]
        bool operator>=(const Vector3& other) const noexcept
        {
            return length() >= other.length();
        }
    };
} // namespace omath

export template<>
struct std::hash<omath::Vector3<float>>
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

export template<class Type>
struct std::formatter<omath::Vector3<Type>> // NOLINT(*-dcl58-cpp)
{
    [[nodiscard]]
    static constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template<class FormatContext>
    [[nodiscard]]
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
