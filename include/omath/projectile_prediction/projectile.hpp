//
// Created by Vlad on 6/9/2024.
//

#pragma once
#include "omath/vector3.hpp"

namespace omath::projectile_prediction
{
    class Projectile final
    {
    public:
        [[nodiscard]]
        Vector3<float> predict_position(float pitch, float yaw, float time, float gravity) const noexcept;

        Vector3<float> m_origin;
        float m_launch_speed{};
        float m_gravity_scale{};
    };
} // namespace omath::projectile_prediction