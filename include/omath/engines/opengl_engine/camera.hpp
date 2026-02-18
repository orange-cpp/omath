//
// Created by Orange on 12/23/2024.
//
#pragma once
#include "omath/engines/opengl_engine/constants.hpp"
#include "omath/projection/camera.hpp"
#include "traits/camera_trait.hpp"

namespace omath::opengl_engine
{
    using Camera =  projection::Camera<Mat4X4, ViewAngles, CameraTrait, true>;
} // namespace omath::opengl_engine