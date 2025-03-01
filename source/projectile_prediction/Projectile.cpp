//
// Created by Vlad on 6/9/2024.
//

#include "omath/projectile_prediction/Projectile.hpp"

#include <omath/engines/Source/Formulas.hpp>

namespace omath::projectile_prediction
{
    Vector3<float> Projectile::PredictPosition(const float pitch, const float yaw, const float time, const float gravity) const
    {
        auto currentPos = m_origin + source::ForwardVector({source::PitchAngle::FromDegrees(-pitch),
                                                            source::YawAngle::FromDegrees(yaw),
                                                            source::RollAngle::FromDegrees(0)}) *
                                             m_launchSpeed * time;
        currentPos.z -= (gravity * m_gravityScale) * (time * time) * 0.5f;

        return currentPos;
    }
} // namespace omath::prediction
