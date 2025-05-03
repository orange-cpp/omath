//
// Created by Vlad on 6/9/2024.
//

#include "omath/projectile_prediction/projectile.hpp"
#include <omath/engines/source_engine/formulas.hpp>

namespace omath::projectile_prediction
{
    Vector3<float> Projectile::predict_position(const float pitch, const float yaw, const float time,
                                                const float gravity) const
    {
        auto current_pos = m_origin
                           + source_engine::forward_vector({source_engine::PitchAngle::from_degrees(-pitch),
                                                            source_engine::YawAngle::from_degrees(yaw),
                                                            source_engine::RollAngle::from_degrees(0)})
                                     * m_launch_speed * time;
        current_pos.z -= (gravity * m_gravity_scale) * (time * time) * 0.5f;

        return current_pos;
    }
} // namespace omath::projectile_prediction
