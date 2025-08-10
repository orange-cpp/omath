//
// Created by Vlad on 3/17/2025.
//

#pragma once
#include "omath/engines/iw_engine/constants.hpp"
#include "omath/projection/camera.hpp"
#include "traits/camera_trait.hpp"

namespace omath::iw_engine
{
    using Camera =  projection::Camera<Mat4X4, ViewAngles, CameraTrait>;
} // namespace omath::iw_engine