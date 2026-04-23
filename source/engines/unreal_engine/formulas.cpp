//
// Created by Vlad on 3/22/2025.
//
#include "omath/engines/unreal_engine/formulas.hpp"
namespace omath::unreal_engine
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
        return mat_camera_view<float, MatStoreType::ROW_MAJOR>(forward_vector(angles), right_vector(angles),
                                                               up_vector(angles), cam_origin);
    }
    Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept
    {
        return mat_rotation_axis_x<float, MatStoreType::ROW_MAJOR>(angles.roll)
               * mat_rotation_axis_z<float, MatStoreType::ROW_MAJOR>(angles.yaw)
               * mat_rotation_axis_y<float, MatStoreType::ROW_MAJOR>(-angles.pitch);
    }


    Mat4X4 calc_perspective_projection_matrix(const float field_of_view, const float aspect_ratio, const float near,
                                              const float far, const NDCDepthRange ndc_depth_range) noexcept
    {
        // UE stores horizontal FOV in FMinimalViewInfo — mirror engine's
        // FMinimalViewInfo::CalculateProjectionMatrixGivenViewRectangle:
        //   XAxisMultiplier = 1 / tan(hfov/2)
        //   YAxisMultiplier = aspect / tan(hfov/2)
        const float inv_tan_half_hfov = 1.f / std::tan(angles::degrees_to_radians(field_of_view) / 2.f);
        const float x_axis = inv_tan_half_hfov;
        const float y_axis = inv_tan_half_hfov * aspect_ratio;

        if (ndc_depth_range == NDCDepthRange::ZERO_TO_ONE)
            return {
                        {x_axis, 0,      0,                     0},
                        {0,      y_axis, 0,                     0},
                        {0,      0,      far / (far - near),   -(near * far) / (far - near)},
                        {0,      0,      1,                     0},
                };
        if (ndc_depth_range == NDCDepthRange::NEGATIVE_ONE_TO_ONE)
            return {
                        {x_axis, 0,      0,                             0},
                        {0,      y_axis, 0,                             0},
                        {0,      0,      (far + near) / (far - near),  -(2.f * far * near) / (far - near)},
                        {0,      0,      1,                             0},
                };
        std::unreachable();
    }
} // namespace omath::unreal_engine
