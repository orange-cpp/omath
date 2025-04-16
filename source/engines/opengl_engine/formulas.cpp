//
// Created by Vlad on 3/19/2025.
//
#include "omath/engines/opengl_engine/formulas.hpp"


namespace omath::opengl_engine
{

    Vector3<float> ForwardVector(const ViewAngles& angles)
    {
        const auto vec = RotationMatrix(angles) * MatColumnFromVector<float, MatStoreType::COLUMN_MAJOR>(kAbsForward);

        return {vec.At(0, 0), vec.At(1, 0), vec.At(2, 0)};
    }
    Vector3<float> RightVector(const ViewAngles& angles)
    {
        const auto vec = RotationMatrix(angles) * MatColumnFromVector<float, MatStoreType::COLUMN_MAJOR>(kAbsRight);

        return {vec.At(0, 0), vec.At(1, 0), vec.At(2, 0)};
    }
    Vector3<float> UpVector(const ViewAngles& angles)
    {
        const auto vec = RotationMatrix(angles) * MatColumnFromVector<float, MatStoreType::COLUMN_MAJOR>(kAbsUp);

        return {vec.At(0, 0), vec.At(1, 0), vec.At(2, 0)};
    }
    Mat4x4 CalcViewMatrix(const ViewAngles& angles, const Vector3<float>& cam_origin)
    {
        return MatCameraView<float, MatStoreType::COLUMN_MAJOR>(-ForwardVector(angles), RightVector(angles),
                                                                UpVector(angles), cam_origin);
    }
    Mat4x4 RotationMatrix(const ViewAngles& angles)
    {
        return MatRotationAxisZ<float, MatStoreType::COLUMN_MAJOR>(angles.roll) *
               MatRotationAxisY<float, MatStoreType::COLUMN_MAJOR>(angles.yaw) *
               MatRotationAxisX<float, MatStoreType::COLUMN_MAJOR>(angles.pitch);
    }
    Mat4x4 CalcPerspectiveProjectionMatrix(const float fieldOfView, const float aspectRatio, const float near,
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
} // namespace omath::opengl_engine
