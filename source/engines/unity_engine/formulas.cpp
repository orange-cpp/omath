//
// Created by Vlad on 3/22/2025.
//
#include "omath/engines/unity_engine/formulas.hpp"



namespace omath::unity_engine
{
    Vector3<float> ForwardVector(const ViewAngles& angles)
    {
        const auto vec = MatRotation(angles) * MatColumnFromVector(kAbsForward);

        return {vec.At(0, 0), vec.At(1, 0), vec.At(2, 0)};
    }
    Vector3<float> RightVector(const ViewAngles& angles)
    {
        const auto vec = MatRotation(angles) * MatColumnFromVector(kAbsRight);

        return {vec.At(0, 0), vec.At(1, 0), vec.At(2, 0)};
    }
    Vector3<float> UpVector(const ViewAngles& angles)
    {
        const auto vec = MatRotation(angles) * MatColumnFromVector(kAbsUp);

        return {vec.At(0, 0), vec.At(1, 0), vec.At(2, 0)};
    }
    Mat4x4 CalcViewMatrix(const ViewAngles& angles, const Vector3<float>& cam_origin)
    {
        return MatCameraView<float, MatStoreType::ROW_MAJOR>(ForwardVector(angles), -RightVector(angles),
                                                             UpVector(angles), cam_origin);
    }
    Mat4x4 CalcPerspectiveProjectionMatrix(const float fieldOfView, const float aspectRatio, const float near,
                                           const float far)
    {
        const float fovHalfTan = std::tan(angles::DegreesToRadians(fieldOfView) / 2.f);

        return {
                {1.f / (aspectRatio * fovHalfTan), 0, 0, 0},
                {0, 1.f / (fovHalfTan), 0, 0},
                {0, 0, (far + near) / (far - near), -(2.f * far * near) / (far - near)},
                {0, 0, -1.f, 0},
        };
    }
} // namespace omath::unity_engine
