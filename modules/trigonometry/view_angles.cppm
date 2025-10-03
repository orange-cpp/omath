//
// Created by Vlad on 10/3/2025.
//
export module omath.trigonometry.view_angles;

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
