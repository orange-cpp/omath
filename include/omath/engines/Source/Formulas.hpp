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
        const auto vec = MatRotation(angles) * MatColumnFromVector(kAbsForward);

        return {vec.At(0, 0), vec.At(1, 0), vec.At(2, 0)};
    }

    [[nodiscard]]
    inline Vector3 RightVector(const ViewAngles& angles)
    {
        const auto vec = MatRotation(angles) * MatColumnFromVector(kAbsRight);

        return {vec.At(0, 0), vec.At(1, 0), vec.At(2, 0)};
    }

    [[nodiscard]]
    inline Vector3 UpVector(const ViewAngles& angles)
    {
        const auto vec = MatRotation(angles) * MatColumnFromVector(kAbsUp);

        return {vec.At(0, 0), vec.At(1, 0), vec.At(2, 0)};
    }

    [[nodiscard]] inline Mat4x4 CalcViewMatrix(const ViewAngles& angles, const Vector3& cam_origin)
    {
        return MatCameraView(ForwardVector(angles), RightVector(angles), UpVector(angles), cam_origin);
    }


    [[nodiscard]]
    inline Mat4x4 CalcPerspectiveProjectionMatrix(const float fieldOfView, const float aspectRatio, const float near,
                                                  const float far)
    {
        // NOTE: Needed tp make thing draw normal, since source is wierd
        // and use tricky projection matrix formula.
        constexpr auto kMultiplyFactor = 0.75f;

        const float fovHalfTan = std::tan(angles::DegreesToRadians(fieldOfView) / 2.f) * kMultiplyFactor;

        return {
                {1.f / (aspectRatio * fovHalfTan), 0, 0, 0},
                {0, 1.f / (fovHalfTan), 0, 0},
                {0, 0, (far + near) / (far - near), -(2.f * far * near) / (far - near)},
                {0, 0, 1, 0},

        };
    }
} // namespace omath::source
