//
// Created by Orange on 12/4/2024.
//
#pragma once
#include "omath/engines/source_engine/constants.hpp"
#include "omath/projection/camera.hpp"

namespace omath::source_engine
{
    class Camera final : public projection::Camera<Mat4x4, ViewAngles>
    {
    public:
        Camera(const Vector3<float>& position, const ViewAngles& viewAngles, const projection::ViewPort& viewPort,
               const Angle<float, 0.f, 180.f, AngleFlags::Clamped>& fov, float near, float far);
        void LookAt(const Vector3<float>& target) override;
    protected:
        [[nodiscard]] Mat4x4 CalcViewMatrix() const override;
        [[nodiscard]] Mat4x4 CalcProjectionMatrix() const override;
    };
}