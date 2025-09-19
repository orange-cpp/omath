//
// Created by Vlad on 8/11/2025.
//
#include "omath/engines/unreal_engine/traits/camera_trait.hpp"

namespace omath::unreal_engine
{

    Mat4X4 CameraTrait::calc_look_at_mat(const Vector3<float>& cam_origin, const Vector3<float>& look_at) noexcept
    {
        return mat_look_at_left_handed<float, Mat4X4::get_store_ordering()>(cam_origin, look_at, k_abs_up);
    }
    Mat4X4 CameraTrait::calc_view_matrix(const ViewAngles& angles, const Vector3<float>& cam_origin) noexcept
    {
        return unreal_engine::calc_view_matrix(angles, cam_origin);
    }
    Mat4X4 CameraTrait::calc_projection_matrix(const projection::FieldOfView& fov,
                                               const projection::ViewPort& view_port, const float near,
                                               const float far) noexcept
    {
        return calc_perspective_projection_matrix(fov.as_degrees(), view_port.aspect_ratio(), near, far);
    }
} // namespace omath::unreal_engine