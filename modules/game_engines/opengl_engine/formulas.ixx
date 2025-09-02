//
// Created by Vlad on 9/2/2025.
//
module;
#include <cmath>
export module omath.opengl_engine.formulas;

import omath.vector3;
import omath.opengl_engine.constants;
import omath.mat;
import omath.angle;

export namespace omath::opengl_engine
{
    [[nodiscard]]
    Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept
    {
        return mat_rotation_axis_x<float, MatStoreType::COLUMN_MAJOR>(-angles.pitch)
               * mat_rotation_axis_y<float, MatStoreType::COLUMN_MAJOR>(-angles.yaw)
               * mat_rotation_axis_z<float, MatStoreType::COLUMN_MAJOR>(angles.roll);
    }

    [[nodiscard]]
    Vector3<float> forward_vector(const ViewAngles& angles) noexcept
    {
        const auto vec =
                rotation_matrix(angles) * mat_column_from_vector<float, MatStoreType::COLUMN_MAJOR>(k_abs_forward);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }

    [[nodiscard]]
    Vector3<float> right_vector(const ViewAngles& angles) noexcept
    {
        const auto vec =
                rotation_matrix(angles) * mat_column_from_vector<float, MatStoreType::COLUMN_MAJOR>(k_abs_right);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }

    [[nodiscard]]
    Vector3<float> up_vector(const ViewAngles& angles) noexcept
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector<float, MatStoreType::COLUMN_MAJOR>(k_abs_up);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }

    [[nodiscard]] Mat4X4 calc_view_matrix(const ViewAngles& angles, const Vector3<float>& cam_origin) noexcept
    {
        return mat_camera_view<float, MatStoreType::COLUMN_MAJOR>(-forward_vector(angles), right_vector(angles),
                                                                  up_vector(angles), cam_origin);
    }

    [[nodiscard]]
    Mat4X4 calc_perspective_projection_matrix(const float field_of_view, const float aspect_ratio, const float near,
                                              const float far) noexcept
    {
        const float fov_half_tan = std::tan(degrees_to_radians(field_of_view) / 2.f);

        return {
                {1.f / (aspect_ratio * fov_half_tan), 0, 0, 0},
                {0, 1.f / (fov_half_tan), 0, 0},
                {0, 0, -(far + near) / (far - near), -(2.f * far * near) / (far - near)},
                {0, 0, -1, 0},
        };
    }
} // namespace omath::opengl_engine
