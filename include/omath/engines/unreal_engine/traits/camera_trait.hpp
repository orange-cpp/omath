//
// Created by Vlad on 8/10/2025.
//

#pragma once
#include "omath/engines/unreal_engine/formulas.hpp"
#include "omath/internal/optional_constexpr_math.hpp"
#include "omath/projection/camera.hpp"

namespace omath::unreal_engine
{
    class CameraTrait final
    {
    public:
        [[nodiscard]]
        OMATH_CONSTEXPR static ViewAngles calc_look_at_angle(const Vector3<double>& cam_origin,
                                                             const Vector3<double>& look_at) noexcept
        {
            const auto direction = (look_at - cam_origin).normalized();

            return {PitchAngle::from_radians(internal::asin(direction.z)),
                    YawAngle::from_radians(internal::atan2(direction.y, direction.x)), RollAngle::from_radians(0.f)};
        }

        [[nodiscard]]
        OMATH_CONSTEXPR static Mat4X4 calc_view_matrix(const ViewAngles& angles,
                                                       const Vector3<double>& cam_origin) noexcept
        {
            return unreal_engine::calc_view_matrix(angles, cam_origin);
        }
        [[nodiscard]]
        OMATH_CONSTEXPR static Mat4X4
        calc_projection_matrix(const projection::FieldOfView& fov, const projection::ViewPort& view_port,
                               const double near, const double far, const NDCDepthRange ndc_depth_range) noexcept
        {
            return calc_perspective_projection_matrix(fov.as_degrees(), view_port.aspect_ratio(), near, far,
                                                      ndc_depth_range);
        }
    };

} // namespace omath::unreal_engine
