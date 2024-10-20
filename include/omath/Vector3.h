//
// Created by vlad on 10/28/23.
//

#pragma once

#include <cstdint>
#include <functional>
#include "omath/Vector2.h"


namespace omath
{
    class Vector3 : public Vector2
    {
    public:
        float z = 0.f;
        constexpr Vector3(float x, float y, float z) : Vector2(x, y), z(z) { }
        constexpr Vector3() : Vector2(), z(0.f) {};

        [[nodiscard]] constexpr bool operator==(const Vector3& src) const
        {
            return Vector2::operator==(src) && (src.z == z);
        }

        [[nodiscard]] constexpr bool operator!=(const Vector3& src) const
        {
            return !(*this == src);
        }

        constexpr Vector3& operator+=(const Vector3& v)
        {
            Vector2::operator+=(v);
            z += v.z;

            return *this;
        }

        constexpr Vector3& operator-=(const Vector3& v)
        {
            Vector2::operator-=(v);
            z -= v.z;

            return *this;
        }

        constexpr Vector3& operator*=(float fl)
        {
            Vector2::operator*=(fl);
            z *= fl;

            return *this;
        }

        constexpr Vector3& operator*=(const Vector3& v)
        {
            Vector2::operator*=(v);
            z *= v.z;

            return *this;
        }

        constexpr Vector3& operator/=(const Vector3& v)
        {
            Vector2::operator/=(v);
            z /= v.z;

            return *this;
        }

        constexpr Vector3& operator+=(float fl)
        {
            Vector2::operator+=(fl);
            z += fl;

            return *this;
        }

        constexpr Vector3& operator/=(float fl)
        {
            Vector2::operator/=(fl);
            z /= fl;

            return *this;
        }

        constexpr Vector3& operator-=(float fl)
        {
            Vector2::operator-=(fl);
            z -= fl;

            return *this;
        }

        constexpr Vector3& Abs()
        {
            Vector2::Abs();
            z = z < 0.f ? -z : z;

            return *this;
        }

        [[nodiscard]] constexpr float DistToSqr(const Vector3& vOther) const
        {
            return (*this - vOther).LengthSqr();
        }

        [[nodiscard]] constexpr float Dot(const Vector3& vOther) const
        {
            return Vector2::Dot(vOther) + z * vOther.z;
        }

#ifndef _MSC_VER
        [[nodiscard]] constexpr float Length() const
        {
            return std::hypot(x, y, z);
        }

        [[nodiscard]] constexpr float Length2D() const
        {
            return Vector2::Length();
        }
        [[nodiscard]] float DistTo(const Vector3& vOther) const
        {
            return (*this - vOther).Length();
        }
        [[nodiscard]] constexpr Vector3 Normalized() const
        {
            const float length = this->Length();

            return length != 0 ? *this / length : *this;
        }
#else
        [[nodiscard]] float Length() const
        {
            return std::hypot(x, y, z);
        }

        [[nodiscard]] Vector3 Normalized() const
        {
            const float length = this->Length();

            return length != 0 ? *this / length : *this;
        }

        [[nodiscard]] float Length2D() const
        {
            return Vector2::Length();
        }

        [[nodiscard]] float DistTo(const Vector3& vOther) const
        {
            return (*this - vOther).Length();
        }
#endif


        [[nodiscard]] constexpr float LengthSqr() const
        {
            return Vector2::LengthSqr() + z * z;
        }

        [[nodiscard]] constexpr Vector3 operator-() const
        {
            return {-x, -y, -z};
        }

        [[nodiscard]] constexpr Vector3 operator+(const Vector3& v) const
        {
            return {x + v.x, y + v.y, z + v.z};
        }

        [[nodiscard]] constexpr Vector3 operator-(const Vector3& v) const
        {
            return {x - v.x, y - v.y, z - v.z};
        }

        [[nodiscard]] constexpr Vector3 operator*(float fl) const
        {
            return {x * fl, y * fl, z * fl};
        }

        [[nodiscard]] constexpr Vector3 operator*(const Vector3& v) const
        {
            return {x * v.x, y * v.y, z * v.z};
        }

        [[nodiscard]] constexpr Vector3 operator/(const float fl) const
        {
            return {x / fl, y / fl, z / fl};
        }

        [[nodiscard]] constexpr Vector3 operator/(const Vector3& v) const
        {
            return {x / v.x, y / v.y, z / v.z};
        }

        [[nodiscard]] constexpr Vector3 Cross(const Vector3 &v) const
        {
            return
            {
                y * v.z - z * v.y,
                z * v.x - x * v.z,
                x * v.y - y * v.x
            };
        }
        [[nodiscard]] constexpr float Sum() const
        {
            return Vector3::Sum2D() + z;
        }

        [[nodiscard]] constexpr float Sum2D() const
        {
            return Vector2::Sum();
        }

        [[nodiscard]] Vector3 ViewAngleTo(const Vector3& other) const;

        [[nodiscard]] static Vector3 ForwardVector(float pitch, float yaw);
        [[nodiscard]] static Vector3 RightVector(float pitch, float yaw, float roll);
        [[nodiscard]] static Vector3 UpVector(float pitch, float yaw, float roll);

        [[nodiscard]] std::tuple<float, float, float> AsTuple() const
        {
            return std::make_tuple(x, y, z);
        }
    };
}
// ReSharper disable once CppRedundantNamespaceDefinition
namespace std
{
    template<>
    struct hash<omath::Vector3>
    {
        std::size_t operator()(const omath::Vector3& vec) const noexcept
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
