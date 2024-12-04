//
// Created by Orange on 12/4/2024.
//
#pragma once
#include "Constants.h"
#include "omath/projection/Camera.hpp"

namespace omath::source
{
    class Camera final : public projection::Camera<Mat4x4, ViewAngles>
    {
    public:
        Camera(const Vector3& position, const ViewAngles& viewAngles,
               const projection::ViewPort& viewPort, const Angle<float, 0, 180, AngleFlags::Clamped>& fov, float near, const float far) :
            projection::Camera<Mat4x4, ViewAngles>(position, viewAngles, viewPort, fov, near, far)
        {
        }
        void LookAt(const Vector3& target) override;
        [[nodiscard]] Mat4x4 GetViewMatrix() const override;
        [[nodiscard]] Mat4x4 GetProjectionMatrix() const override;
    };
}