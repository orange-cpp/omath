//
// Created by Vlad on 3/17/2025.
//

#pragma once
#include "Constants.hpp"

namespace omath::iw_engine
{
    [[nodiscard]]
    inline Vector3<float> ForwardVector(const ViewAngles& angles)
    {
        const auto vec = MatRotation(angles) * MatColumnFromVector(kAbsForward);

        return {vec.At(0, 0), vec.At(1, 0), vec.At(2, 0)};
    }

    [[nodiscard]]
    inline Vector3<float> RightVector(const ViewAngles& angles)
    {
        const auto vec = MatRotation(angles) * MatColumnFromVector(kAbsRight);

        return {vec.At(0, 0), vec.At(1, 0), vec.At(2, 0)};
    }

    [[nodiscard]]
    inline Vector3<float> UpVector(const ViewAngles& angles)
    {
        const auto vec = MatRotation(angles) * MatColumnFromVector(kAbsUp);

        return {vec.At(0, 0), vec.At(1, 0), vec.At(2, 0)};
    }

    [[nodiscard]] inline Mat4x4 CalcViewMatrix(const ViewAngles& angles, const Vector3<float>& cam_origin)
    {
        return MatCameraView(ForwardVector(angles), RightVector(angles), UpVector(angles), cam_origin);
    }


    [[nodiscard]]
    inline Mat4x4 CalcPerspectiveProjectionMatrix(const float fieldOfView, const float aspectRatio, const float near,
                                                  const float far)
    {
        const float fovHalfTan = std::tan(angles::DegreesToRadians(fieldOfView) / 2.f) * 0.75f;

        return
        {
            {1.f / (aspectRatio * fovHalfTan), 0, 0, 0},
            {0, 1.f / fovHalfTan, 0, 0},
            {0, 0, (far + near) / (far - near), -(2.f * far * near) / (far - near)},
            {0, 0, 1, 0},
        };
    }
} // namespace omath::source
