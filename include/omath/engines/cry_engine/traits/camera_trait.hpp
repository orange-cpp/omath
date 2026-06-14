//
// Created by Vlad on 8/10/2025.
//

#pragma once
#include "omath/engines/cry_engine/formulas.hpp"
#include "omath/internal/constexpr_math.hpp"
#include "omath/projection/camera.hpp"
namespace omath::cry_engine
{
    class CameraTrait final
    {
    public:
        [[nodiscard]]
        constexpr static ViewAngles calc_look_at_angle(const Vector3<float>& cam_origin,
                                                       const Vector3<float>& look_at) noexcept
        {
            const auto direction = (look_at - cam_origin).normalized();
            return {PitchAngle::from_radians(internal::asin(direction.z)),
                    YawAngle::from_radians(-internal::atan2(direction.x, direction.y)), RollAngle::from_radians(0.f)};
        }

        [[nodiscard]]
        constexpr static Mat4X4 calc_view_matrix(const ViewAngles& angles, const Vector3<float>& cam_origin) noexcept
        {
            return cry_engine::calc_view_matrix(angles, cam_origin);
        }
        [[nodiscard]]
        constexpr static Mat4X4 calc_projection_matrix(const projection::FieldOfView& fov,
                                                       const projection::ViewPort& view_port, const float near_plane,
                                                       const float far_plane,
                                                       const NDCDepthRange ndc_depth_range) noexcept
        {
            return calc_perspective_projection_matrix(fov.as_degrees(), view_port.aspect_ratio(), near_plane, far_plane,
                                                      ndc_depth_range);
        }
    };

} // namespace omath::cry_engine
