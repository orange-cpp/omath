//
// Created by Vlad on 3/19/2025.
//
#include "omath/engines/iw_engine/formulas.hpp"

namespace omath::iw_engine
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
    Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept
    {
        return mat_rotation_axis_z(angles.yaw) * mat_rotation_axis_y(angles.pitch) * mat_rotation_axis_x(angles.roll);
    }

    Mat4X4 calc_view_matrix(const ViewAngles& angles, const Vector3<float>& cam_origin) noexcept
    {
        return mat_camera_view(forward_vector(angles), right_vector(angles), up_vector(angles), cam_origin);
    }

    Mat4X4 calc_perspective_projection_matrix(const float field_of_view, const float aspect_ratio, const float near,
                                              const float far, const NDCDepthRange ndc_depth_range) noexcept
    {
        // InfinityWard Engine (inherited from Quake) stores FOV as horizontal FOV at a 4:3
        // reference aspect. Convert to true vertical FOV, then delegate to the
        // standard vertical-FOV left-handed builder with the caller's actual
        // aspect ratio.
        //   vfov = 2 · atan( tan(hfov_4:3 / 2) / (4/3) )
        constexpr float k_source_reference_aspect = 4.f / 3.f;
        const float half_hfov_4_3 = angles::degrees_to_radians(field_of_view) / 2.f;
        const float vfov_deg = angles::radians_to_degrees(
                2.f * std::atan(std::tan(half_hfov_4_3) / k_source_reference_aspect));

        if (ndc_depth_range == NDCDepthRange::ZERO_TO_ONE)
            return mat_perspective_left_handed<
                    float, MatStoreType::ROW_MAJOR, NDCDepthRange::ZERO_TO_ONE>(
                    vfov_deg, aspect_ratio, near, far);
        if (ndc_depth_range == NDCDepthRange::NEGATIVE_ONE_TO_ONE)
            return mat_perspective_left_handed<
                    float, MatStoreType::ROW_MAJOR, NDCDepthRange::NEGATIVE_ONE_TO_ONE>(
                    vfov_deg, aspect_ratio, near, far);
        std::unreachable();
    };
} // namespace omath::iw_engine
