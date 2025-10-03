//
// Created by Vlad on 03.09.2024.
//
module;
#include <cstdint>

export module omath.projection.error_code;

export namespace omath::projection
{
    enum class Error : std::uint16_t
    {
        WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS,
        INV_VIEW_PROJ_MAT_DET_EQ_ZERO,
    };
}