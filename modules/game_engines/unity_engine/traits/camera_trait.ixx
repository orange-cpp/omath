//
// Created by Vlad on 9/3/2025.
//
module;
#include <cmath>

export module omath.unity_engine.camera_trait;
import omath.unity_engine.formulas;
import omath.projectile_prediction.projectile;
import omath.projectile_prediction.target;
import omath.camera;

export namespace omath::unity_engine
{
    class CameraTrait final
    {
    public:
        [[nodiscard]]
        static ViewAngles calc_look_at_angle(const Vector3<float>& cam_origin, const Vector3<float>& look_at) noexcept
        {
            const auto distance = cam_origin.distance_to(look_at);
            const auto delta = cam_origin - look_at;

            return {PitchAngle::from_radians(-std::asin(delta.y / distance)),
                    YawAngle::from_radians(std::atan2(delta.z, delta.x)), RollAngle::from_radians(0.f)};
        }

        [[nodiscard]]
        static Mat4X4 calc_view_matrix(const ViewAngles& angles, const Vector3<float>& cam_origin) noexcept
        {
            return unity_engine::calc_view_matrix(angles, cam_origin);
        }

        [[nodiscard]]
        static Mat4X4 calc_projection_matrix(const projection::FieldOfView& fov, const projection::ViewPort& view_port,
                                             const float near, const float far) noexcept
        {
            return calc_perspective_projection_matrix(fov.as_degrees(), view_port.aspect_ratio(), near, far);
        }
    };

}