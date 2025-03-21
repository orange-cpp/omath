//
// Created by Vlad on 6/9/2024.
//

#include "omath/projectile_prediction/projectile.hpp"

#include <omath/engines/source_engine/formulas.hpp>

namespace omath::projectile_prediction
{
    Vector3<float> Projectile::PredictPosition(const float pitch, const float yaw, const float time, const float gravity) const
    {
        auto currentPos = m_origin + source_engine::ForwardVector({source_engine::PitchAngle::FromDegrees(-pitch),
                                                            source_engine::YawAngle::FromDegrees(yaw),
                                                            source_engine::RollAngle::FromDegrees(0)}) *
                                             m_launchSpeed * time;
        currentPos.z -= (gravity * m_gravityScale) * (time * time) * 0.5f;

        return currentPos;
    }
} // namespace omath::prediction
