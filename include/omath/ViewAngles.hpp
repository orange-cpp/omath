//
// Created by Orange on 11/30/2024.
//
#pragma once

namespace omath
{
    template<class PitchType, class YawType, class RollType>
    struct ViewAngles
    {
        PitchType pitch;
        YawType yaw;
        RollType roll;
    };
}
