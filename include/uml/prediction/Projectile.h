//
// Created by Vlad on 6/9/2024.
//

#pragma once
#include "uml/Vector3.h"


namespace uml::prediction
{
    class Projectile final
    {
    public:

        [[nodiscard]]
        Vector3 CalculateVelocity(float pitch, float yaw) const;

        [[nodiscard]]
        Vector3 PredictPosition(float pitch, float yaw, float time, float gravity) const;

        Vector3 m_origin;
        float m_launchSpeed{};
        float m_gravityScale{};
    };
}