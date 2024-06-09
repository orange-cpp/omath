//
// Created by Vlad on 6/9/2024.
//

#include "uml/prediction/Projectile.h"
#include <cmath>


namespace uml::prediction
{
    Vector3 Projectile::CalculateVelocity(const float pitch, const float yaw) const
    {
        return Vector3::CreateVelocity(pitch, yaw, m_launchSpeed);
    }

    Vector3 Projectile::PredictPosition(const float pitch, const float yaw, const float time, const float gravity) const
    {
        auto currentPos = m_origin + Vector3::CreateVelocity(pitch, yaw, m_launchSpeed) * time;
        currentPos.z -= (gravity * m_gravityScale) * std::pow(time, 2.f) * 0.5f;

        return currentPos;
    }
}
