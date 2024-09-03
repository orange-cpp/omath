//
// Created by vlad on 10/28/23.
//

#include <omath/Vector3.h>
#include <cmath>
#include <omath/Angles.h>

namespace omath
{

    float Vector3::DistTo(const Vector3 &vOther) const
    {
        return (*this - vOther).Length();
    }

    float Vector3::Length() const
    {
        return std::sqrt(Vector2::LengthSqr() + z * z);
    }


    float Vector3::Length2D() const
    {
        return Vector2::Length();
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


        return {sinRoll*sinPitch*cosYaw + cosRoll*sinYaw,
                sinRoll*sinPitch*sinYaw + -cosRoll*cosYaw,
                -sinRoll*cosPitch};
    }

    Vector3 Vector3::UpVector(float pitch, float yaw, float roll)
    {
        return RightVector(pitch, yaw, roll).Cross(ForwardVector(pitch, yaw));
    }


    Vector3 Vector3::Normalized() const
    {
        const float length = this->Length();

        return length != 0 ? *this / length : *this;
    }
}