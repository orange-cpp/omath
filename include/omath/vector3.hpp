//
// Created by vlad on 10/28/23.
//

#pragma once

#include <cstdint>
#include <expected>
#include <functional>
#include "omath/angle.hpp"
#include "omath/vector2.hpp"

namespace omath
{

    enum class Vector3Error
    {
        IMPOSSIBLE_BETWEEN_ANGLE,
    };

    template<class Type> requires std::is_arithmetic_v<Type>
    class Vector3 : public Vector2<Type>
    {
    public:
        Type z = static_cast<Type>(0);
        constexpr Vector3(const Type& x, const Type& y, const Type& z) : Vector2<Type>(x, y), z(z) { }
        constexpr Vector3() : Vector2<Type>() {};

        [[nodiscard]] constexpr bool operator==(const Vector3& src) const
        {
            return Vector2<Type>::operator==(src) && (src.z == z);
        }

        [[nodiscard]] constexpr bool operator!=(const Vector3& src) const
        {
            return !(*this == src);
        }

        constexpr Vector3& operator+=(const Vector3& v)
        {
            Vector2<Type>::operator+=(v);
            z += v.z;

            return *this;
        }

        constexpr Vector3& operator-=(const Vector3& v)
        {
            Vector2<Type>::operator-=(v);
            z -= v.z;

            return *this;
        }

        constexpr Vector3& operator*=(const float fl)
        {
            Vector2<Type>::operator*=(fl);
            z *= fl;

            return *this;
        }

        constexpr Vector3& operator*=(const Vector3& v)
        {
            Vector2<Type>::operator*=(v);
            z *= v.z;

            return *this;
        }

        constexpr Vector3& operator/=(const Vector3& v)
        {
            Vector2<Type>::operator/=(v);
            z /= v.z;

            return *this;
        }

        constexpr Vector3& operator+=(const float fl)
        {
            Vector2<Type>::operator+=(fl);
            z += fl;

            return *this;
        }

        constexpr Vector3& operator/=(const float fl)
        {
            Vector2<Type>::operator/=(fl);
            z /= fl;

            return *this;
        }

        constexpr Vector3& operator-=(const float fl)
        {
            Vector2<Type>::operator-=(fl);
            z -= fl;

            return *this;
        }

        constexpr Vector3& Abs()
        {
            Vector2<Type>::Abs();
            z = z < 0.f ? -z : z;

            return *this;
        }

        [[nodiscard]] constexpr Type DistToSqr(const Vector3& vOther) const
        {
            return (*this - vOther).LengthSqr();
        }

        [[nodiscard]] constexpr Type Dot(const Vector3& vOther) const
        {
            return Vector2<Type>::Dot(vOther) + z * vOther.z;
        }

#ifndef _MSC_VER
        [[nodiscard]] constexpr Type Length() const
        {
            return std::hypot(this->x, this->y, z);
        }

        [[nodiscard]] constexpr Type Length2D() const
        {
            return Vector2<Type>::Length();
        }
        [[nodiscard]] Type DistTo(const Vector3& vOther) const
        {
            return (*this - vOther).Length();
        }
        [[nodiscard]] constexpr Vector3 Normalized() const
        {
            const Type length = this->Length();

            return length != 0 ? *this / length : *this;
        }
#else
        [[nodiscard]] Type Length() const
        {
            return std::hypot(this->x, this->y, z);
        }

        [[nodiscard]] Vector3 Normalized() const
        {
            const Type length = this->Length();

            return length != 0 ? *this / length : *this;
        }

        [[nodiscard]] Type Length2D() const
        {
            return Vector2<Type>::Length();
        }

        [[nodiscard]] Type DistTo(const Vector3& vOther) const
        {
            return (*this - vOther).Length();
        }
#endif


        [[nodiscard]] constexpr Type LengthSqr() const
        {
            return Vector2<Type>::LengthSqr() + z * z;
        }

        [[nodiscard]] constexpr Vector3 operator-() const
        {
            return {-this->x, -this->y, -z};
        }

        [[nodiscard]] constexpr Vector3 operator+(const Vector3& v) const
        {
            return {this->x + v.x, this->y + v.y, z + v.z};
        }

        [[nodiscard]] constexpr Vector3 operator-(const Vector3& v) const
        {
            return {this->x - v.x, this->y - v.y, z - v.z};
        }

        [[nodiscard]] constexpr Vector3 operator*(const float fl) const
        {
            return {this->x * fl, this->y * fl, z * fl};
        }

        [[nodiscard]] constexpr Vector3 operator*(const Vector3& v) const
        {
            return {this->x * v.x, this->y * v.y, z * v.z};
        }

        [[nodiscard]] constexpr Vector3 operator/(const float fl) const
        {
            return {this->x / fl, this->y / fl, z / fl};
        }

        [[nodiscard]] constexpr Vector3 operator/(const Vector3& v) const
        {
            return {this->x / v.x, this->y / v.y, z / v.z};
        }

        [[nodiscard]] constexpr Vector3 Cross(const Vector3 &v) const
        {
            return
            {
                this->y * v.z - z * v.y,
                z * v.x - this->x * v.z,
                this->x * v.y - this->y * v.x
            };
        }
        [[nodiscard]] constexpr Type Sum() const
        {
            return Sum2D() + z;
        }

        [[nodiscard]] std::expected<Angle<float, 0.f, 180.f, AngleFlags::Clamped>, Vector3Error>
        AngleBetween(const Vector3& other) const
        {
            const auto bottom = Length() * other.Length();

            if (bottom == 0.f)
                return std::unexpected(Vector3Error::IMPOSSIBLE_BETWEEN_ANGLE);

            return Angle<float, 0.f, 180.f, AngleFlags::Clamped>::FromRadians(std::acos(Dot(other) / bottom));
        }

        [[nodiscard]] bool IsPerpendicular(const Vector3& other) const
        {
            if (const auto angle = AngleBetween(other))
                return angle->AsDegrees() == 90.f;

            return false;
        }

        [[nodiscard]] constexpr Type Sum2D() const
        {
            return Vector2<Type>::Sum();
        }

        [[nodiscard]] constexpr std::tuple<Type, Type, Type> AsTuple() const
        {
            return std::make_tuple(this->x, this->y, z);
        }

        [[nodiscard]] Vector3 ViewAngleTo(const Vector3 &other) const
        {
            const float distance = DistTo(other);
            const auto delta = other - *this;

            return
            {
                angles::RadiansToDegrees(std::asin(delta.z / distance)),
                angles::RadiansToDegrees(std::atan2(delta.y, delta.x)),
                0
            };
        }
    };
}
// ReSharper disable once CppRedundantNamespaceDefinition
namespace std
{
    template<>
    struct hash<omath::Vector3<float>>
    {
        std::size_t operator()(const omath::Vector3<float>& vec) const noexcept
        {
            std::size_t hash = 0;
            constexpr std::hash<float> hasher;

            hash ^= hasher(vec.x) + 0x9e3779b9 + (hash<<6) + (hash>>2);
            hash ^= hasher(vec.y) + 0x9e3779b9 + (hash<<6) + (hash>>2);
            hash ^= hasher(vec.z) + 0x9e3779b9 + (hash<<6) + (hash>>2);

            return hash;
        }
    };
}
