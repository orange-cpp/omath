//
// Created by Vlad on 3/19/2025.
//
#include <omath/engines/source_engine/formulas.hpp>

namespace omath::source_engine
{
    Vector3<float> forward_vector(const ViewAngles& angles)
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector(k_abs_forward);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }

    Mat4X4 rotation_matrix(const ViewAngles& angles)
    {
        return mat_rotation_axis_z(angles.yaw) * mat_rotation_axis_y(angles.pitch) * mat_rotation_axis_x(angles.roll);
    }

    Vector3<float> right_vector(const ViewAngles& angles)
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector(k_abs_right);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }
    Vector3<float> up_vector(const ViewAngles& angles)
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector(k_abs_up);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }

    Mat4X4 calc_view_matrix(const ViewAngles& angles, const Vector3<float>& cam_origin)
    {
        return mat_camera_view(forward_vector(angles), right_vector(angles), up_vector(angles), cam_origin);
    }

    Mat4X4 calc_perspective_projection_matrix(const float field_of_view, const float aspect_ratio, const float near,
                                              const float far)
    {
        // NOTE: Need magic number to fix fov calculation, since source inherit Quake proj matrix calculation
        constexpr auto k_multiply_factor = 0.75f;

        const float fov_half_tan = std::tan(angles::degrees_to_radians(field_of_view) / 2.f) * k_multiply_factor;

        return {
                {1.f / (aspect_ratio * fov_half_tan), 0, 0, 0},
                {0, 1.f / (fov_half_tan), 0, 0},
                {0, 0, (far + near) / (far - near), -(2.f * far * near) / (far - near)},
                {0, 0, 1, 0},
        };
    }
} // namespace omath::source_engine
