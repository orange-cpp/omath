//
// Created by Vlad on 6/9/2024.
//

#pragma once
#include "omath/Vector3.hpp"

namespace omath::prediction
{
    class Target final
    {
    public:

        [[nodiscard]]
        constexpr Vector3 PredictPosition(const float time, const float gravity) const
        {
            auto predicted = m_origin + m_velocity * time;

            if (m_isAirborne)
                predicted.z -= gravity * std::pow(time, 2.f) * 0.5f;

            return predicted;
        }

        Vector3 m_origin;
        Vector3 m_velocity;
        bool m_isAirborne{};
    };
}