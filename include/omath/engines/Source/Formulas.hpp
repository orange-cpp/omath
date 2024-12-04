//
// Created by Orange on 12/4/2024.
//
#pragma once
#include "Constants.h"


namespace omath::source
{
    [[nodiscard]]
    inline Vector3 ForwardVector(const ViewAngles& angles)
    {
        const auto cosPitch = angles.pitch.Cos();
        const auto sinPitch = angles.pitch.Sin();

        const auto cosYaw = angles.yaw.Cos();
        const auto sinYaw = angles.yaw.Sin();


        return {cosPitch * cosYaw, cosPitch * sinYaw, -sinPitch};
    }

    [[nodiscard]]
    inline Vector3 RightVector(const ViewAngles& angles)
    {
        const auto cosPitch = angles.pitch.Cos();
        const auto sinPitch = angles.pitch.Sin();

        const auto cosYaw = angles.yaw.Cos();
        const auto sinYaw = angles.yaw.Sin();

        const auto cosRoll = angles.roll.Cos();
        const auto sinRoll = angles.roll.Sin();


        return
        {
            -1 * sinRoll * sinPitch * cosYaw + -1 * cosRoll * -sinYaw,
            -1 * sinRoll * sinPitch * sinYaw + -1 * cosRoll * cosYaw,
            -1 * sinRoll * cosPitch
        };
    }

    [[nodiscard]]
    inline Vector3 UpVector(const ViewAngles& angles)
    {
        const auto cosPitch = angles.pitch.Cos();
        const auto sinPitch = angles.pitch.Sin();

        const auto cosYaw = angles.yaw.Cos();
        const auto sinYaw = angles.yaw.Sin();

        const auto cosRoll = angles.roll.Cos();
        const auto sinRoll = angles.roll.Sin();


        return
        {
            cosRoll * sinPitch * cosYaw + - sinRoll * -sinYaw,
            cosRoll * sinPitch * sinYaw + - sinRoll * cosYaw,
            cosRoll * cosPitch,
        };
    }

    [[nodiscard]]
    constexpr Mat4x4 ViewMatrixFromVecs(const Vector3& forward, const Vector3& right, const Vector3& up,
                                        const Vector3& camera_pos)
    {
        return Mat4x4{
                           {right.x, right.y, right.z, 0},
                           {up.x, up.y, up.z, 0},
                           {forward.x, forward.y, forward.z, 0},
                           {0, 0, 0, 1},
                   } *
                   MatTranslation<float, MatStoreType::ROW_MAJOR>(-camera_pos);
    }

    [[nodiscard]] inline Mat4x4 ViewMatrix(const ViewAngles& angles, const Vector3& cam_origin)
    {
        return ViewMatrixFromVecs(ForwardVector(angles), RightVector(angles), UpVector(angles), cam_origin);
    }


    [[nodiscard]]
    inline Mat4x4 PerspectiveProjectionMatrix(const float fieldOfView, const float aspectRatio, const float near, const float far)
    {
        constexpr auto kMultiplyFactor = 0.75f;

        const float fovHalfTan = std::tan(angles::DegreesToRadians(fieldOfView) / 2.f) * kMultiplyFactor;

        return {
                        {-1.f / (aspectRatio * fovHalfTan), 0, 0, 0},
                        {0, -1.f / (fovHalfTan), 0, 0},
                        {0, 0, (far + near) / (far - near), -(2.f * far * near) / (far - near)},
                        {0, 0, 1, 0},
                };
    }
} // namespace omath::source
