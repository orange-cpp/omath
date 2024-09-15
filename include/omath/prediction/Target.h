//
// Created by Vlad on 6/9/2024.
//

#pragma once
#include "omath/Vector3.h"


namespace omath::prediction
{
    class Target final
    {
    public:

        [[nodiscard]]
        constexpr Vector3 PredictPosition(float time, float gravity) const
        {
            auto predicted = m_origin + m_velocity * time;

            if (m_isAirborne)
#if OMATH_COORDINATE_SYSTEM == OMATH_QUAKE_SUPPORT
                predicted.z -= gravity * (time*time) * 0.5f;
#elif OMATH_COORDINATE_SYSTEM == OMATH_UE_SUPPORT
                predicted.z -= gravity * (time*time) * 0.5f;
#elif OMATH_COORDINATE_SYSTEM == OMATH_UNITY_SUPPORT
                predicted.y -= gravity * (time*time) * 0.5f;
#endif
            return predicted;
        }

        Vector3 m_origin;
        Vector3 m_velocity;
        bool m_isAirborne{};
    };
}