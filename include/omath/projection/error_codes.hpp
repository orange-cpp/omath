//
// Created by Vlad on 03.09.2024.
//

#pragma once
#include <cstdint>

namespace omath::projection
{
    enum class Error : uint16_t
    {
        WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS,
    };
}