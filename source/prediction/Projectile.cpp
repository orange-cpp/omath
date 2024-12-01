//
// Created by Vlad on 6/9/2024.
//

#include "omath/prediction/Projectile.hpp"
#include <cmath>
#include <omath/engines/Source.hpp>

namespace omath::prediction
{
    Vector3 Projectile::PredictPosition(const float pitch, const float yaw, const float time, const float gravity) const
    {
        auto currentPos = m_origin + omath::source::ForwardVector({source::PitchAngle::FromDegrees(pitch), source::YawAngle::FromDegrees(yaw), source::RollAngle::FromDegrees(0)}) * m_launchSpeed * time;
        currentPos.z -= (gravity * m_gravityScale) * std::pow(time, 2.f) * 0.5f;

        return currentPos;
    }
}
