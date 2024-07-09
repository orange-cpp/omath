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
        Vector3 PredictPosition(float time, float gravity) const;

        Vector3 m_origin;
        Vector3 m_velocity;
        bool m_isAirborne{};
    };
}