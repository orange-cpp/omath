//
// Created by vlad on 10/28/23.
//

#include <omath/Vector3.h>
#include <cmath>
#include <omath/angles.h>

namespace omath
{
    bool Vector3::operator==(const Vector3 &src) const
    {
        return (src.x == x) and (src.y == y) and (src.z == z);
    }

    bool Vector3::operator!=(const Vector3 &src) const
    {
        return (src.x != x) or (src.y != y) or (src.z != z);
    }

    Vector3 &Vector3::operator+=(const Vector3 &v)
    {
        x += v.x;
        y += v.y;
        z += v.z;

        return *this;
    }

    Vector3 &Vector3::operator-=(const Vector3 &v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;

        return *this;
    }

    Vector3 &Vector3::operator*=(const float fl)
    {
        x *= fl;
        y *= fl;
        z *= fl;

        return *this;
    }

    Vector3 &Vector3::operator*=(const Vector3 &v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;

        return *this;
    }

    Vector3 &Vector3::operator/=(const Vector3 &v)
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;

        return *this;
    }

    Vector3 &Vector3::operator+=(const float fl)
    {
        x += fl;
        y += fl;
        z += fl;

        return *this;
    }

    Vector3 &Vector3::operator/=(const float fl)
    {
        x /= fl;
        y /= fl;
        z /= fl;

        return *this;
    }

    Vector3 &Vector3::operator-=(const float fl)
    {
        x -= fl;
        y -= fl;
        z -= fl;

        return *this;
    }

    float Vector3::DistTo(const Vector3 &vOther) const
    {
        Vector3 delta;

        delta.x = x - vOther.x;
        delta.y = y - vOther.y;
        delta.z = z - vOther.z;

        return delta.Length();
    }

    Vector3 &Vector3::Abs()
    {
        x = std::abs(x);
        y = std::abs(y);
        z = std::abs(z);

        return *this;
    }

    float Vector3::DistToSqr(const Vector3 &vOther) const
    {
        Vector3 delta;

        delta.x = x - vOther.x;
        delta.y = y - vOther.y;
        delta.z = z - vOther.z;

        return delta.LengthSqr();
    }

    float Vector3::Dot(const Vector3 &vOther) const
    {
        return (x * vOther.x + y * vOther.y + z * vOther.z);
    }

    float Vector3::Length() const
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    float Vector3::LengthSqr() const
    {
        return (x * x + y * y + z * z);
    }

    float Vector3::Length2D() const
    {
        return std::sqrt(x * x + y * y);

    }

    Vector3 Vector3::operator-() const
    {
        return {-x, -y, -z};
    }

    Vector3 Vector3::operator+(const Vector3 &v) const
    {
        return {x + v.x, y + v.y, z + v.z};
    }

    Vector3 Vector3::operator-(const Vector3 &v) const
    {
        return {x - v.x, y - v.y, z - v.z};
    }

    Vector3 Vector3::operator*(float fl) const
    {
        return {x * fl, y * fl, z * fl};
    }

    Vector3 Vector3::operator*(const Vector3 &v) const
    {
        return {x * v.x, y * v.y, z * v.z};
    }

    Vector3 Vector3::operator/(const float fl) const
    {
        return {x / fl, y / fl, z / fl};
    }

    Vector3 Vector3::operator/(const Vector3 &v) const
    {
        return {x / v.x, y / v.y, z / v.z};
    }

    Vector3 Vector3::CreateVelocity(const float pitch, const float yaw, const float speed)
    {
        return
        {
            std::cos(angles::DegreesToRadians(pitch)) * std::cos(angles::DegreesToRadians(yaw)) * speed,
            std::cos(angles::DegreesToRadians(pitch)) * std::sin(angles::DegreesToRadians(yaw)) * speed,
            std::sin(angles::DegreesToRadians(pitch)) * speed,
        };
    }

    float Vector3::Sum() const
    {
        return x + y + z;
    }

    float Vector3::Sum2D() const
    {
        return x + y;
    }

    Vector3 Vector3::ViewAngleTo(const Vector3 &other) const
    {
        const float distance = DistTo(other);
        const auto delta = other - *this;

        return
        {
            angles::RadiansToDegrees(asinf(delta.z / distance)),
            angles::RadiansToDegrees(atan2f(delta.y, delta.x)),
            0.f
        };
    }

    Vector3 Vector3::ForwardVector(const float pitch, const float yaw)
    {
        const auto cosPitch = std::cos(angles::DegreesToRadians(pitch));
        const auto sinPitch = std::sin(angles::DegreesToRadians(pitch));

        const auto cosYaw = std::cos(angles::DegreesToRadians(yaw));
        const auto sinYaw = std::sin(angles::DegreesToRadians(yaw));


        return {cosPitch*cosYaw, cosPitch*sinYaw, sinPitch};
    }

    Vector3 Vector3::RightVector(const float pitch, const float yaw, const float roll)
    {
        const auto cosPitch = std::cos(angles::DegreesToRadians(pitch));
        const auto sinPitch = std::sin(angles::DegreesToRadians(pitch));

        const auto cosYaw = std::cos(angles::DegreesToRadians(yaw));
        const auto sinYaw = std::sin(angles::DegreesToRadians(yaw));

        const auto cosRoll = std::cos(angles::DegreesToRadians(yaw));
        const auto sinRoll = std::sin(angles::DegreesToRadians(yaw));


        return {-sinRoll*sinPitch*cosYaw + -cosRoll*-sinYaw,
                -sinRoll*sinPitch*sinYaw + -cosRoll*cosYaw,
                sinRoll*cosPitch};
    }

    Vector3 Vector3::Cross(const Vector3 &v) const
    {
        return {
                y * v.z - z * v.y,
                z * v.x - x * v.z,
                x * v.y - y * v.x
        };
    }

    Vector3 Vector3::Normalized() const
    {
        const float length = this->Length();
        if (length != 0)
        {
            return *this / length;
        }
        return *this;
    }
}