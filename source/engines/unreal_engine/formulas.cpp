//
// Created by Vlad on 3/22/2025.
//
#include "omath/engines/unreal_engine/formulas.hpp"
namespace omath::unreal_engine
{
    Vector3<double> forward_vector(const ViewAngles& angles) noexcept
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector(k_abs_forward);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }
    Vector3<double> right_vector(const ViewAngles& angles) noexcept
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector(k_abs_right);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }
    Vector3<double> up_vector(const ViewAngles& angles) noexcept
    {
        const auto vec = rotation_matrix(angles) * mat_column_from_vector(k_abs_up);

        return {vec.at(0, 0), vec.at(1, 0), vec.at(2, 0)};
    }
    Mat4X4 calc_view_matrix(const ViewAngles& angles, const Vector3<double>& cam_origin) noexcept
    {
        return mat_camera_view<double, MatStoreType::ROW_MAJOR>(forward_vector(angles), right_vector(angles),
                                                               up_vector(angles), cam_origin);
    }
    Mat4X4 rotation_matrix(const ViewAngles& angles) noexcept
    {
        // UE FRotator is intrinsic Z-Y-X (Yaw → Pitch → Roll applied in local
        // frame), which for column-vector composition is Rz·Ry·Rx.
        // Pitch and roll axes in omath spin opposite to UE's convention, so
        // both carry a sign flip.
        return mat_rotation_axis_z<double, MatStoreType::ROW_MAJOR>(angles.yaw)
               * mat_rotation_axis_y<double, MatStoreType::ROW_MAJOR>(-angles.pitch)
               * mat_rotation_axis_x<double, MatStoreType::ROW_MAJOR>(-angles.roll);
    }


    Mat4X4 calc_perspective_projection_matrix(const double field_of_view, const double aspect_ratio, const double near,
                                              const double far, const NDCDepthRange ndc_depth_range) noexcept
    {
        // UE stores horizontal FOV in FMinimalViewInfo — use the left-handed
        // horizontal-FOV builder directly.
        if (ndc_depth_range == NDCDepthRange::ZERO_TO_ONE)
            return mat_perspective_left_handed_horizontal_fov<
                    double, MatStoreType::ROW_MAJOR, NDCDepthRange::ZERO_TO_ONE>(
                    field_of_view, aspect_ratio, near, far);
        if (ndc_depth_range == NDCDepthRange::NEGATIVE_ONE_TO_ONE)
            return mat_perspective_left_handed_horizontal_fov<
                    double, MatStoreType::ROW_MAJOR, NDCDepthRange::NEGATIVE_ONE_TO_ONE>(
                    field_of_view, aspect_ratio, near, far);
        std::unreachable();
    }
} // namespace omath::unreal_engine
