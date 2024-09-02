//
// Created by vlad on 10/28/23.
//

#include <omath/Vector3.h>
#include <cmath>
#include <omath/angles.h>

namespace omath
{

    Vector3::Vector3(const float x, const float y, const float z) : Vector2(x, y), z(z)
    {

    }

    bool Vector3::operator==(const Vector3 &src) const
    {
        return Vector2::operator==(src) && (src.z == z);
    }

    bool Vector3::operator!=(const Vector3 &src) const
    {
        return !(*this == src);
    }

    Vector3 &Vector3::operator+=(const Vector3 &v)
    {
        Vector2::operator+=(v);
        z += v.z;

        return *this;
    }

    Vector3 &Vector3::operator-=(const Vector3 &v)
    {
        Vector2::operator-=(v);
        z -= v.z;

        return *this;
    }

    Vector3 &Vector3::operator*=(const float fl)
    {
        Vector2::operator*=(fl);
        z *= fl;

        return *this;
    }

    Vector3 &Vector3::operator*=(const Vector3 &v)
    {
        Vector2::operator*=(v);
        z *= v.z;

        return *this;
    }

    Vector3 &Vector3::operator/=(const Vector3 &v)
    {
        Vector2::operator/=(v);
        z /= v.z;

        return *this;
    }

    Vector3 &Vector3::operator+=(const float fl)
    {
        Vector2::operator+=(fl);
        z += fl;

        return *this;
    }

    Vector3 &Vector3::operator/=(const float fl)
    {
        Vector2::operator/=(fl);
        z /= fl;

        return *this;
    }

    Vector3 &Vector3::operator-=(const float fl)
    {
        Vector2::operator-=(fl);
        z -= fl;

        return *this;
    }

    float Vector3::DistTo(const Vector3 &vOther) const
    {
        return (*this - vOther).Length();
    }

    Vector3 &Vector3::Abs()
    {
        Vector2::Abs();
        z = std::abs(z);

        return *this;
    }

    float Vector3::DistToSqr(const Vector3 &vOther) const
    {
        return (*this - vOther).LengthSqr();
    }

    float Vector3::Dot(const Vector3 &vOther) const
    {
        return Vector2::Dot(vOther) + z * vOther.z;
    }

    float Vector3::Length() const
    {
        return std::sqrt(Vector2::LengthSqr() + z * z);
    }

    float Vector3::LengthSqr() const
    {
        return Vector2::LengthSqr() + z * z;
    }

    float Vector3::Length2D() const
    {
        return Vector2::Length();
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
        return Vector3::Sum2D() + z;
    }

    float Vector3::Sum2D() const
    {
        return Vector2::Sum();
    }

    Vector3 Vector3::ViewAngleTo(const Vector3 &other) const
    {
        const float distance = DistTo(other);
        const auto delta = other - *this;

        return
        {
            angles::RadiansToDegrees(std::asin(delta.z / distance)),
            angles::RadiansToDegrees(std::atan2(delta.y, delta.x)),
            0.f
        };
    }

    Vector3 Vector3::ForwardVector(const float pitch, const float yaw)
    {
        const auto cosPitch = std::cos(angles::DegreesToRadians(pitch));
        const auto sinPitch = std::sin(angles::DegreesToRadians(pitch));

        const auto cosYaw = std::cos(angles::DegreesToRadians(yaw));
        const auto sinYaw = std::sin(angles::DegreesToRadians(yaw));


        return
        {
            cosPitch*cosYaw,
            cosPitch*sinYaw,
            sinPitch
        };
    }

    Vector3 Vector3::RightVector(const float pitch, const float yaw, const float roll)
    {
        const auto cosPitch = std::cos(angles::DegreesToRadians(pitch));
        const auto sinPitch = std::sin(angles::DegreesToRadians(pitch));

        const auto cosYaw = std::cos(angles::DegreesToRadians(yaw));
        const auto sinYaw = std::sin(angles::DegreesToRadians(yaw));

        const auto cosRoll = std::cos(angles::DegreesToRadians(roll));
        const auto sinRoll = std::sin(angles::DegreesToRadians(roll));


        return {-sinRoll*sinPitch*cosYaw + -cosRoll*-sinYaw,
                -sinRoll*sinPitch*sinYaw + -cosRoll*cosYaw,
                sinRoll*cosPitch};
    }

    Vector3 Vector3::UpVector(float pitch, float yaw, float roll)
    {
        return RightVector(pitch, yaw, roll).Cross(ForwardVector(pitch, yaw));
    }

    Vector3 Vector3::Cross(const Vector3 &v) const
    {
        return 
        {
                y * v.z - z * v.y,
                z * v.x - x * v.z,
                x * v.y - y * v.x
        };
    }

    Vector3 Vector3::Normalized() const
    {
        const float length = this->Length();

        return length != 0 ? *this / length : *this;
    }

    std::tuple<float, float, float> Vector3::AsTuple() const
    {
        return std::make_tuple(x, y, z);
    }
}