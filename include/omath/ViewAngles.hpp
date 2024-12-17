//
// Created by Orange on 11/30/2024.
//
#pragma once

#include "omath/omath_export.hpp"

namespace omath
{
    template<class PitchType, class YawType, class RollType>
    struct OMATH_API ViewAngles
    {
        PitchType pitch;
        YawType yaw;
        RollType roll;
    };
}
