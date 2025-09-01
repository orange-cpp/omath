//
// Created by Vlad on 9/1/2025.
//

export module omath.view_angles;

export namespace omath
{
    template<class PitchType, class YawType, class RollType>
    struct ViewAngles
    {
        PitchType pitch;
        YawType yaw;
        RollType roll;
    };
} // namespace omath
