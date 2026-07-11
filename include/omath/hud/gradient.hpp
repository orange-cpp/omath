#pragma once
#include "omath/utility/color.hpp"

namespace omath::hud
{
    struct Gradient
    {
        Color top_left;
        Color top_right;
        Color bottom_right;
        Color bottom_left;
    };
} // namespace omath::hud
