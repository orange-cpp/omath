//
// Created by Vlad on 3/19/2025.
//
#include "omath/engines/opengl_engine/formulas.hpp"

namespace omath::opengl_engine
{

    Vector3<float> forward_vector(const ViewAngles& angles) noexcept
    {
        const auto vec =
                rotation_matrix(angles) * mat_column_from_vector<float, MatStoreType::COLUMN_MAJOR>(k_abs_forward);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }
    Vector3<float> right_vector(const ViewAngles& angles) noexcept
    {
        const auto vec =
                rotation_matrix(angles) * mat_column_from_vector<float, MatStoreType::COLUMN_MAJOR>(k_abs_right);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }
    Vector3<float> up_vector(const ViewAngles& angles) noexcept
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector<float, MatStoreType::COLUMN_MAJOR>(k_abs_up);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }
    Mat4X4 calc_view_matrix(const ViewAngles& angles, const Vector3<float>& cam_origin) noexcept
    {
        return mat_look_at_right_handed(cam_origin, cam_origin + forward_vector(angles), up_vector(angles));
    }
    Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept
    {
        return mat_rotation_axis_z<float, MatStoreType::COLUMN_MAJOR>(angles.roll)
               * mat_rotation_axis_y<float, MatStoreType::COLUMN_MAJOR>(angles.yaw)
               * mat_rotation_axis_x<float, MatStoreType::COLUMN_MAJOR>(angles.pitch);
    }
    Mat4X4 calc_perspective_projection_matrix(const float field_of_view, const float aspect_ratio, const float near,
                                              const float far, const NDCDepthRange ndc_depth_range) noexcept
    {
        if (ndc_depth_range == NDCDepthRange::NEGATIVE_ONE_TO_ONE)
            return mat_perspective_right_handed<float, MatStoreType::COLUMN_MAJOR, NDCDepthRange::NEGATIVE_ONE_TO_ONE>(
                    field_of_view, aspect_ratio, near, far);

        if (ndc_depth_range == NDCDepthRange::ZERO_TO_ONE)
            return mat_perspective_right_handed<float, MatStoreType::COLUMN_MAJOR, NDCDepthRange::ZERO_TO_ONE>(
                        field_of_view, aspect_ratio, near, far);

        std::unreachable();
    }
} // namespace omath::opengl_engine
