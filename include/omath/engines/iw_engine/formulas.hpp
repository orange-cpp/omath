//
// Created by Vlad on 3/17/2025.
//

#pragma once
#include "omath/engines/iw_engine/constants.hpp"

namespace omath::iw_engine
{
    [[nodiscard]]
    Vector3<float> forward_vector(const ViewAngles& angles);

    [[nodiscard]]
    Vector3<float> right_vector(const ViewAngles& angles);

    [[nodiscard]]
    Vector3<float> up_vector(const ViewAngles& angles);

    [[nodiscard]]
    Mat4X4 rotation_matrix(const ViewAngles& angles);

    [[nodiscard]] Mat4X4 calc_view_matrix(const ViewAngles& angles, const Vector3<float>& cam_origin);

    [[nodiscard]]
    Mat4X4 calc_perspective_projection_matrix(float field_of_view, float aspect_ratio, float near, float far);
} // namespace omath::iw_engine
