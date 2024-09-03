//
// Created by Vlad on 6/9/2024.
//

#include "omath/prediction/Projectile.h"
#include <cmath>


namespace omath::prediction
{
    Vector3 Projectile::PredictPosition(const float pitch, const float yaw, const float time, const float gravity) const
    {
        auto currentPos = m_origin + Vector3::ForwardVector(pitch, yaw) * m_launchSpeed * time;
        currentPos.z -= (gravity * m_gravityScale) * std::pow(time, 2.f) * 0.5f;

        return currentPos;
    }
}
