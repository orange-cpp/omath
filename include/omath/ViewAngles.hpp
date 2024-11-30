//
// Created by Orange on 11/30/2024.
//
#pragma once
#include <type_traits>
#include <__algorithm/clamp.h>

#include "omath/Angles.hpp"


namespace omath
{
    template<class Type, Type min = 0, Type max = 360>
    requires std::is_arithmetic_v<Type>
    class ViewAngles
    {
        Type pitch;
        Type yaw;
        Type roll;

        constexpr void SetPitch(const Type& newPitch)
        {
            pitch = std::clamp(newPitch, min, max);
        }
        void SetYaw(const Type& newYaw)
        {
            yaw = std::clamp(newYaw, min, max);
        }
        void SetRoll(const Type& newRoll)
        {
            roll = angles::WrapAngle(newRoll, min, max);
        }

    };
}
