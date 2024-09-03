//
// Created by Vlad on 6/9/2024.
//

#pragma once
#include "omath/Vector3.h"


namespace omath::prediction
{
    class Projectile final
    {
    public:

        [[nodiscard]]
        Vector3 PredictPosition(float pitch, float yaw, float time, float gravity) const;

        Vector3 m_origin;
        float m_launchSpeed{};
        float m_gravityScale{};
    };
}