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
#if OMATH_COORDINATE_SYSTEM == OMATH_QUAKE_SUPPORT
            angles::RadiansToDegrees(std::asin(delta.z / distance)),
            angles::RadiansToDegrees(std::atan2(delta.y, delta.x)),
#elif OMATH_COORDINATE_SYSTEM == OMATH_UE_SUPPORT
            angles::RadiansToDegrees(std::asin(delta.z / distance)),
            angles::RadiansToDegrees(std::atan2(delta.y, delta.x)),
#elif OMATH_COORDINATE_SYSTEM == OMATH_UNITY_SUPPORT
            angles::RadiansToDegrees(std::asin(delta.y / distance)),
            angles::RadiansToDegrees(std::atan2(delta.z, delta.x)),
#elif OMATH_COORDINATE_SYSTEM == OMATH_OPENGL_SUPPORT
    angles::RadiansToDegrees(std::asin(delta.y / distance)),
    angles::RadiansToDegrees(std::atan2(delta.z, delta.x)),
#else
#   error "Coordinate system is not supported"
#endif
            0.f
        };
    }
#if OMATH_COORDINATE_SYSTEM == OMATH_QUAKE_SUPPORT
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
#elif OMATH_COORDINATE_SYSTEM == OMATH_UE_SUPPORT
    Vector3 Vector3::ForwardVector(const float pitch, const float yaw)
    {
        const auto radPitch = angles::DegreesToRadians(pitch);
        const auto radYaw = angles::DegreesToRadians(yaw);

        const auto cosPitch = std::cos(radPitch);
        const auto sinPitch = std::sin(radPitch);

        const auto cosYaw = std::cos(radYaw);
        const auto sinYaw = std::sin(radYaw);

        return
        {
            cosPitch * cosYaw,   // X component
            cosPitch * sinYaw,   // Y component
            sinPitch             // Z component
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
            sinRoll * sinPitch * cosYaw + cosRoll * sinYaw,
            sinRoll * sinPitch * sinYaw - cosRoll * cosYaw,
            -sinRoll * cosPitch
        };
    }
#elif OMATH_COORDINATE_SYSTEM == OMATH_UNITY_SUPPORT
    Vector3 Vector3::ForwardVector(const float pitch, const float yaw)
    {
        const auto radPitch = angles::DegreesToRadians(pitch);
        const auto radYaw = angles::DegreesToRadians(yaw);

        const auto cosPitch = std::cos(radPitch);
        const auto sinPitch = std::sin(radPitch);

        const auto cosYaw = std::cos(radYaw);
        const auto sinYaw = std::sin(radYaw);

        return
        {
            cosPitch * sinYaw,
            sinPitch,
            cosPitch * cosYaw
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
            cosRoll * cosYaw + sinRoll * sinPitch * sinYaw,
            sinRoll * cosPitch,
            cosRoll * -sinYaw + sinRoll * sinPitch * cosYaw
        };
    }
#elif  OMATH_COORDINATE_SYSTEM == OMATH_OPENGL_SUPPORT
    Vector3 Vector3::ForwardVector(const float pitch, const float yaw)
    {
        const auto radPitch = angles::DegreesToRadians(pitch);
        const auto radYaw = angles::DegreesToRadians(yaw);

        const auto cosPitch = std::cos(radPitch);
        const auto sinPitch = std::sin(radPitch);

        const auto cosYaw = std::cos(radYaw);
        const auto sinYaw = std::sin(radYaw);

        return
        {
            -sinYaw * cosPitch, // X component
            sinPitch,           // Y component
            -cosYaw * cosPitch  // Z component
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
            cosRoll * cosYaw * cosPitch + sinRoll * sinPitch * sinYaw,   // X component
            sinRoll * cosPitch,                                          // Y component
            -cosRoll * sinYaw * cosPitch + sinRoll * sinPitch * cosYaw   // Z component
        };
    }
#endif

    Vector3 Vector3::UpVector(float pitch, float yaw, float roll)
    {
#if OMATH_COORDINATE_SYSTEM == OMATH_UNITY_SUPPORT
        return ForwardVector(pitch, yaw).Cross(RightVector(pitch, yaw, roll));

#elif OMATH_COORDINATE_SYSTEM == OMATH_QUAKE_SUPPORT
        return RightVector(pitch, yaw,roll).Cross(ForwardVector(pitch, yaw));

#elif OMATH_COORDINATE_SYSTEM == OMATH_UE_SUPPORT
        return ForwardVector(pitch, yaw).Cross(RightVector(pitch, yaw, roll));

#elif OMATH_COORDINATE_SYSTEM == OMATH_OPENGL_SUPPORT
        return RightVector(pitch, yaw,roll).Cross(ForwardVector(pitch, yaw));
#endif
    }

    Vector3 Vector3::Normalized() const
    {
        const float length = this->Length();

        return length != 0 ? *this / length : *this;
    }
}