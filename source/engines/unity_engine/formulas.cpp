//
// Created by Vlad on 3/22/2025.
//
#include "omath/engines/unity_engine/formulas.hpp"

namespace omath::unity_engine
{
    Vector3<float> forward_vector(const ViewAngles& angles) noexcept
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector(k_abs_forward);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }
    Vector3<float> right_vector(const ViewAngles& angles) noexcept
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector(k_abs_right);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }
    Vector3<float> up_vector(const ViewAngles& angles) noexcept
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector(k_abs_up);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }
    Mat4X4 calc_view_matrix(const ViewAngles& angles, const Vector3<float>& cam_origin) noexcept
    {
        return mat_camera_view<float, MatStoreType::ROW_MAJOR>(-forward_vector(angles), right_vector(angles),
                                                               up_vector(angles), cam_origin);
    }
    Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept
    {
        return mat_rotation_axis_z<float, MatStoreType::ROW_MAJOR>(angles.roll)
               * mat_rotation_axis_y<float, MatStoreType::ROW_MAJOR>(angles.yaw)
               * mat_rotation_axis_x<float, MatStoreType::ROW_MAJOR>(angles.pitch);
    }
    Mat4X4 calc_perspective_projection_matrix(const float field_of_view, const float aspect_ratio, const float near,
                                              const float far) noexcept
    {
        return omath::mat_perspective_right_handed(field_of_view, aspect_ratio, near, far);
    }
} // namespace omath::unity_engine
