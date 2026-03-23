//
// Created by Vlad on 3/22/2025.
//

#pragma once
#include "omath/engines/unity_engine/constants.hpp"
#include "omath/projection/camera.hpp"
#include "traits/camera_trait.hpp"

namespace omath::unity_engine
{
    using Camera = projection::Camera<Mat4X4, ViewAngles, CameraTrait, false, NDCDepthRange::ZERO_TO_ONE>;
} // namespace omath::unity_engine