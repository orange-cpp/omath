//
// Created by Vlad on 9/1/2025.
//
module;
#include <cmath>
export module omath.iw_engine.camera_trait;

import omath.iw_engine.constants;
import omath.vector3;
import omath.camera;
import omath.iw_engine.formulas;

export namespace omath::iw_engine
{
    class CameraTrait final
    {
    public:
        [[nodiscard]]
        static ViewAngles calc_look_at_angle(const Vector3<float>& cam_origin, const Vector3<float>& look_at) noexcept
        {
            const auto distance = cam_origin.distance_to(look_at);
            const auto delta = cam_origin - look_at;

            return {PitchAngle::from_radians(-std::asin(delta.z / distance)),
                    YawAngle::from_radians(std::atan2(delta.y, delta.x)), RollAngle::from_radians(0.f)};
        }

        [[nodiscard]]
        static Mat4X4 calc_view_matrix(const ViewAngles& angles, const Vector3<float>& cam_origin) noexcept
        {
            return iw_engine::calc_view_matrix(angles, cam_origin);
        }

        [[nodiscard]]
        static Mat4X4 calc_projection_matrix(const projection::FieldOfView& fov, const projection::ViewPort& view_port,
                                             const float near, const float far) noexcept
        {
            return calc_perspective_projection_matrix(fov.as_degrees(), view_port.aspect_ratio(), near, far);
        }
    };

} // namespace omath::iw_engine