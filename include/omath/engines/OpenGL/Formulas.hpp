//
// Created by Orange on 12/23/2024.
//
#pragma once
#include "Constants.hpp"


namespace omath::opengl
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
        return MatCameraView<float, MatStoreType::COLUMN_MAJOR>(-ForwardVector(angles), RightVector(angles), UpVector(angles), cam_origin);
    }


    [[nodiscard]]
    inline Mat4x4 CalcPerspectiveProjectionMatrix(const float fieldOfView, const float aspectRatio, const float near,
                                                  const float far)
    {
        const float fovHalfTan = std::tan(angles::DegreesToRadians(fieldOfView) / 2.f);

        return {
                    {1.f / (aspectRatio * fovHalfTan), 0, 0, 0},
                    {0, 1.f / (fovHalfTan), 0, 0},
                    {0, 0, -(far + near) / (far - near), -(2.f * far * near) / (far - near)},
                    {0, 0, -1, 0},

            };
    }
}