//
// Created by Vlad on 9/1/2025.
//
module;
#include <cstdint>

export module omath.error_codes;


export namespace omath::projection
{
    enum class Error : uint16_t
    {
        WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS,
    };
}