//
// Created by Orange on 12/23/2024.
//
#pragma once
#include "omath/engines/opengl_engine/constants.hpp"
#include "omath/projection/camera.hpp"

namespace omath::opengl_engine
{
    class Camera final : public projection::Camera<Mat4X4, ViewAngles>
    {
    public:
        Camera(const Vector3<float>& position, const ViewAngles& view_angles, const projection::ViewPort& view_port,
               const Angle<float, 0.f, 180.f, AngleFlags::Clamped>& fov, float near, float far);
        void look_at(const Vector3<float>& target) override;
        [[nodiscard]] Mat4X4 calc_view_matrix() const noexcept override;
        [[nodiscard]] Mat4X4 calc_projection_matrix() const noexcept override;
    };
} // namespace omath::opengl_engine