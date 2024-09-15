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
#if OMATH_COORDINATE_SYSTEM == OMATH_QUAKE_SUPPORT
        currentPos.z -= (gravity * m_gravityScale) * (time*time) * 0.5f;
#elif OMATH_COORDINATE_SYSTEM == OMATH_UE_SUPPORT
        currentPos.z -= (gravity * m_gravityScale) * (time*time) * 0.5f;
#elif OMATH_COORDINATE_SYSTEM == OMATH_UNITY_SUPPORT
        currentPos.y -= (gravity * m_gravityScale) * (time*time) * 0.5f;
#endif
        return currentPos;
    }
}
