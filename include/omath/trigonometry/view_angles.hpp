//
// Created by Orange on 11/30/2024.
//
#pragma once
#include "omath/linear_algebra/vector3.hpp"
#include <type_traits>
namespace omath
{
    template<class PitchType, class YawType, class RollType>
    requires std::is_same_v<typename PitchType::ArithmeticType, typename YawType::ArithmeticType>
             && std::is_same_v<typename YawType::ArithmeticType, typename RollType::ArithmeticType>
    struct ViewAngles
    {
        using ArithmeticType = PitchType::ArithmeticType;

        PitchType pitch;
        YawType yaw;
        RollType roll;

        [[nodiscard]]
        Vector3<ArithmeticType> as_vector3() const
        {
            return {pitch.as_degrees(), yaw.as_degrees(), roll.as_degrees()};
        }
    };
} // namespace omath
