//
// Created by vlad on 10/28/23.
//

#include <omath/Vector3.hpp>
#include <cmath>
#include <omath/Angles.hpp>

namespace omath
{
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
        const auto radPitch = angles::DegreesToRadians(pitch);
        const auto radYaw = angles::DegreesToRadians(yaw);
        const auto radRoll = angles::DegreesToRadians(roll);

        const auto cosPitch = std::cos(radPitch);
        const auto sinPitch = std::sin(radPitch);

        const auto cosYaw = std::cos(radYaw);
        const auto sinYaw = std::sin(radYaw);

        const auto cosRoll = std::cos(radRoll);
        const auto sinRoll = std::sin(radRoll);


        return
        {
            sinRoll*sinPitch*cosYaw + cosRoll*sinYaw,
            sinRoll*sinPitch*sinYaw - cosRoll*cosYaw,
            -sinRoll*cosPitch
        };
    }

    Vector3 Vector3::UpVector(float pitch, float yaw, float roll)
    {
        return RightVector(pitch, yaw, roll).Cross(ForwardVector(pitch, yaw));
    }
}