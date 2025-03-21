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
        Vector3<float> PredictPosition(float pitch, float yaw, float time, float gravity) const;

        Vector3<float> m_origin;
        float m_launchSpeed{};
        float m_gravityScale{};
    };
}