//
// Created by Vlad on 8/11/2025.
//
#include "omath/engines/source_engine/traits/camera_trait.hpp"
#include "omath/engines/source_engine/formulas.hpp"
namespace omath::source_engine
{

    ViewAngles CameraTrait::calc_look_at_angle(const Vector3<float>& cam_origin, const Vector3<float>& look_at) noexcept
    {
        const auto direction = (look_at - cam_origin).normalized();


        return {PitchAngle::from_radians(-std::asin(direction.z)),
                YawAngle::from_radians(std::atan2(direction.y, direction.x)), RollAngle::from_radians(0.f)};
    }
    Mat4X4 CameraTrait::calc_view_matrix(const ViewAngles& angles, const Vector3<float>& cam_origin) noexcept
    {
        return source_engine::calc_view_matrix(angles, cam_origin);
    }
    Mat4X4 CameraTrait::calc_projection_matrix(const projection::FieldOfView& fov,
                                               const projection::ViewPort& view_port, const float near,
                                               const float far) noexcept
    {
        return calc_perspective_projection_matrix(fov.as_degrees(), view_port.aspect_ratio(), near, far);
    }
} // namespace omath::source_engine