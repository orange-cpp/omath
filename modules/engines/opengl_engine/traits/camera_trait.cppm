//
// Created by Vlad on 8/10/2025.
//

#pragma once
#include "omath/engines/opengl_engine/constants.hpp"
#include "omath/projection/camera.hpp"

namespace omath::opengl_engine
{
    class CameraTrait final
    {
    public:
        [[nodiscard]]
        static ViewAngles calc_look_at_angle(const Vector3<float>& cam_origin, const Vector3<float>& look_at) noexcept;

        [[nodiscard]]
        static Mat4X4 calc_view_matrix(const ViewAngles& angles, const Vector3<float>& cam_origin) noexcept;
        [[nodiscard]]
        static Mat4X4 calc_projection_matrix(const projection::FieldOfView& fov, const projection::ViewPort& view_port,
                                             float near, float far) noexcept;
    };

} // namespace omath::opengl_engine