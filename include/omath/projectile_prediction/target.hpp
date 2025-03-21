//
// Created by Vlad on 6/9/2024.
//

#pragma once
#include "omath/vector3.hpp"

namespace omath::projectile_prediction
{
    class Target final
    {
    public:

        [[nodiscard]]
        constexpr Vector3<float> PredictPosition(const float time, const float gravity) const
        {
            auto predicted = m_origin + m_velocity * time;

            if (m_isAirborne)
                predicted.z -= gravity * std::pow(time, 2.f) * 0.5f;

            return predicted;
        }

        Vector3<float> m_origin;
        Vector3<float> m_velocity;
        bool m_isAirborne{};
    };
}