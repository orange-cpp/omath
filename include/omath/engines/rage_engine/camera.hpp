//
// Created by Orange on 6/3/2026.
//

#pragma once
#include "omath/engines/rage_engine/constants.hpp"
#include "omath/projection/camera.hpp"
#include "traits/camera_trait.hpp"

namespace omath::rage_engine
{
    using Camera = projection::Camera<Mat4X4, ViewAngles, CameraTrait, NDCDepthRange::ZERO_TO_ONE>;
} // namespace omath::rage_engine
