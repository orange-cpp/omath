//
// Created by vlad on 3/1/2026.
//
#pragma once

#include "omath/linear_algebra/mat.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <array>
#include <cmath>
#include <format>

namespace omath
{
    template<class Type>
    requires std::is_arithmetic_v<Type>
    class Quaternion
    {
    public:
        using ContainedType = Type;

        Type x = static_cast<Type>(0);
        Type y = static_cast<Type>(0);
        Type z = static_cast<Type>(0);
        Type w = static_cast<Type>(1);  // identity quaternion

        constexpr Quaternion() noexcept = default;

        constexpr Quaternion(const Type& x, const Type& y, const Type& z, const Type& w) noexcept
            : x(x), y(y), z(z), w(w)
        {
        }

        // Factory: build from a normalized axis and an angle in radians
        [[nodiscard]]
        static Quaternion from_axis_angle(const Vector3<Type>& axis, const Type& angle_rad) noexcept
        {
            const Type half = angle_rad / static_cast<Type>(2);
            const Type s = std::sin(half);
            return {axis.x * s, axis.y * s, axis.z * s, std::cos(half)};
        }

        [[nodiscard]] constexpr bool operator==(const Quaternion& other) const noexcept
        {
            return x == other.x && y == other.y && z == other.z && w == other.w;
        }

        [[nodiscard]] constexpr bool operator!=(const Quaternion& other) const noexcept
        {
            return !(*this == other);
        }

        // Hamilton product: this * other
        [[nodiscard]] constexpr Quaternion operator*(const Quaternion& other) const noexcept
        {
            return {
                w * other.x + x * other.w + y * other.z - z * other.y,
                w * other.y - x * other.z + y * other.w + z * other.x,
                w * other.z + x * other.y - y * other.x + z * other.w,
                w * other.w - x * other.x - y * other.y - z * other.z,
            };
        }

        constexpr Quaternion& operator*=(const Quaternion& other) noexcept
        {
            return *this = *this * other;
        }

        [[nodiscard]] constexpr Quaternion operator*(const Type& scalar) const noexcept
        {
            return {x * scalar, y * scalar, z * scalar, w * scalar};
        }

        constexpr Quaternion& operator*=(const Type& scalar) noexcept
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            w *= scalar;
            return *this;
        }

        [[nodiscard]] constexpr Quaternion operator+(const Quaternion& other) const noexcept
        {
            return {x + other.x, y + other.y, z + other.z, w + other.w};
        }

        constexpr Quaternion& operator+=(const Quaternion& other) noexcept
        {
            x += other.x;
            y += other.y;
            z += other.z;
            w += other.w;
            return *this;
        }

        [[nodiscard]] constexpr Quaternion operator-() const noexcept
        {
            return {-x, -y, -z, -w};
        }

        // Conjugate: negates the vector part (x, y, z)
        [[nodiscard]] constexpr Quaternion conjugate() const noexcept
        {
            return {-x, -y, -z, w};
        }

        [[nodiscard]] constexpr Type dot(const Quaternion& other) const noexcept
        {
            return x * other.x + y * other.y + z * other.z + w * other.w;
        }

        [[nodiscard]] constexpr Type length_sqr() const noexcept
        {
            return x * x + y * y + z * z + w * w;
        }

#ifndef _MSC_VER
        [[nodiscard]] constexpr Type length() const noexcept
        {
            return std::sqrt(length_sqr());
        }

        [[nodiscard]] constexpr Quaternion normalized() const noexcept
        {
            const Type len = length();
            return len != static_cast<Type>(0) ? *this * (static_cast<Type>(1) / len) : *this;
        }
#else
        [[nodiscard]] Type length() const noexcept
        {
            return std::sqrt(length_sqr());
        }

        [[nodiscard]] Quaternion normalized() const noexcept
        {
            const Type len = length();
            return len != static_cast<Type>(0) ? *this * (static_cast<Type>(1) / len) : *this;
        }
#endif

        // Inverse: q* / |q|^2  (for unit quaternions inverse == conjugate)
        [[nodiscard]] constexpr Quaternion inverse() const noexcept
        {
            return conjugate() * (static_cast<Type>(1) / length_sqr());
        }

        // Rotate a 3D vector: v' = q * pure(v) * q^-1
        // Computed via Rodrigues' formula to avoid full quaternion product overhead
        [[nodiscard]] constexpr Vector3<Type> rotate(const Vector3<Type>& v) const noexcept
        {
            const Vector3<Type> q_vec{x, y, z};
            const Vector3<Type> cross = q_vec.cross(v);
            return v + cross * (static_cast<Type>(2) * w) + q_vec.cross(cross) * static_cast<Type>(2);
        }

        // 3x3 rotation matrix from this (unit) quaternion
        [[nodiscard]] constexpr Mat<3, 3, Type> to_rotation_matrix3() const noexcept
        {
            const Type xx = x * x, yy = y * y, zz = z * z;
            const Type xy = x * y, xz = x * z, yz = y * z;
            const Type wx = w * x, wy = w * y, wz = w * z;
            const Type one = static_cast<Type>(1);
            const Type two = static_cast<Type>(2);

            return {
                {one - two * (yy + zz), two * (xy - wz),       two * (xz + wy)      },
                {two * (xy + wz),       one - two * (xx + zz), two * (yz - wx)      },
                {two * (xz - wy),       two * (yz + wx),       one - two * (xx + yy)},
            };
        }

        // 4x4 rotation matrix (with homogeneous row/column)
        [[nodiscard]] constexpr Mat<4, 4, Type> to_rotation_matrix4() const noexcept
        {
            const Type xx = x * x, yy = y * y, zz = z * z;
            const Type xy = x * y, xz = x * z, yz = y * z;
            const Type wx = w * x, wy = w * y, wz = w * z;
            const Type one = static_cast<Type>(1);
            const Type two = static_cast<Type>(2);
            const Type zero = static_cast<Type>(0);

            return {
                {one - two * (yy + zz), two * (xy - wz),       two * (xz + wy),       zero},
                {two * (xy + wz),       one - two * (xx + zz), two * (yz - wx),       zero},
                {two * (xz - wy),       two * (yz + wx),       one - two * (xx + yy), zero},
                {zero,                  zero,                   zero,                  one },
            };
        }

        [[nodiscard]] constexpr std::array<Type, 4> as_array() const noexcept
        {
            return {x, y, z, w};
        }
    };
} // namespace omath

template<class Type>
struct std::formatter<omath::Quaternion<Type>> // NOLINT(*-dcl58-cpp)
{
    [[nodiscard]]
    static constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template<class FormatContext>
    [[nodiscard]]
    static auto format(const omath::Quaternion<Type>& q, FormatContext& ctx)
    {
        if constexpr (std::is_same_v<typename FormatContext::char_type, char>)
            return std::format_to(ctx.out(), "[{}, {}, {}, {}]", q.x, q.y, q.z, q.w);

        if constexpr (std::is_same_v<typename FormatContext::char_type, wchar_t>)
            return std::format_to(ctx.out(), L"[{}, {}, {}, {}]", q.x, q.y, q.z, q.w);

        if constexpr (std::is_same_v<typename FormatContext::char_type, char8_t>)
            return std::format_to(ctx.out(), u8"[{}, {}, {}, {}]", q.x, q.y, q.z, q.w);
    }
};
