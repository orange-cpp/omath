//
// Created by Orange on 12/4/2024.
//
#pragma once
#include "Constants.hpp"

namespace omath::source_engine
{
    [[nodiscard]]
    Vector3<float> ForwardVector(const ViewAngles& angles);

    [[nodiscard]]
    Vector3<float> RightVector(const ViewAngles& angles);

    [[nodiscard]]
    Vector3<float> UpVector(const ViewAngles& angles);

    [[nodiscard]] Mat4x4 CalcViewMatrix(const ViewAngles& angles, const Vector3<float>& cam_origin);


    [[nodiscard]]
    Mat4x4 CalcPerspectiveProjectionMatrix(float fieldOfView, float aspectRatio, float near, float far);
} // namespace omath::source
