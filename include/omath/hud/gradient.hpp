#pragma once
#include "omath/utility/color.hpp"

namespace omath::hud
{
    enum class GradientDirection
    {
        RightToLeft,
        LeftToRight,
    };

    struct Gradient final
    {
        Color top_left;
        Color top_right;
        Color bottom_right;
        Color bottom_left;
        bool animated = false;
        float animation_speed = 4.f;
        float animation_spread = 0.7f;
        GradientDirection direction = GradientDirection::RightToLeft;
    };
} // namespace omath::hud
