//
// Created by Vlad on 8/10/2025.
//

#pragma once
#include "omath/engines/unreal_engine/formulas.hpp"
#include "omath/projection/camera.hpp"

namespace omath::unreal_engine
{
    class CameraTrait final
    {
    public:
        [[nodiscard]]
        static ViewAngles calc_look_at_angle(const Vector3<double>& cam_origin, const Vector3<double>& look_at) noexcept;

        [[nodiscard]]
        static Mat4X4 calc_view_matrix(const ViewAngles& angles, const Vector3<double>& cam_origin) noexcept;
        [[nodiscard]]
        static Mat4X4 calc_projection_matrix(const projection::FieldOfView& fov, const projection::ViewPort& view_port,
                                             double near, double far, NDCDepthRange ndc_depth_range) noexcept;
    };

} // namespace omath::unreal_engine