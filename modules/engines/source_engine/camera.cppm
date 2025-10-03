//
// Created by Orange on 12/4/2024.
//
#pragma once
#include "omath/engines/source_engine/constants.hpp"
#include "omath/projection/camera.hpp"
#include "traits/camera_trait.hpp"
namespace omath::source_engine
{
    using Camera =  projection::Camera<Mat4X4, ViewAngles, CameraTrait>;
} // namespace omath::source_engine