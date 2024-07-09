//
// Created by Vlad on 6/9/2024.
//

#include "omath/prediction/Target.h"
#include <cmath>


namespace omath::prediction
{
    Vector3 Target::PredictPosition(const float time, const float gravity) const
    {
        auto predicted = m_origin + m_velocity * time;

        if (m_isAirborne)
            predicted.z -= gravity * std::pow(time, 2.f) * 0.5f;

        return predicted;
    }
}
