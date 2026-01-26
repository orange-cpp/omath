//
// Created by Vlad on 3/22/2025.
//

#pragma once
#include "omath/engines/unity_engine/constants.hpp"
#include "omath/projection/camera.hpp"
#include "traits/camera_trait.hpp"

namespace omath::unity_engine
{
    using Camera = projection::Camera<Mat4X4, ViewAngles, CameraTrait>;
} // namespace omath::unity_engine