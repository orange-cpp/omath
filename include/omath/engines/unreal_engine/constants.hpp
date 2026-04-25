//
// Created by Vlad on 3/22/2025.
//

#pragma once

#include "omath/linear_algebra/mat.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <omath/trigonometry/angle.hpp>
#include <omath/trigonometry/view_angles.hpp>

namespace omath::unreal_engine
{
    constexpr Vector3<double> k_abs_up = {0, 0, 1};
    constexpr Vector3<double> k_abs_right = {0, 1, 0};
    constexpr Vector3<double> k_abs_forward = {1, 0, 0};

    using Mat4X4 = Mat<4, 4, double, MatStoreType::ROW_MAJOR>;
    using Mat3X3 = Mat<4, 4, double, MatStoreType::ROW_MAJOR>;
    using Mat1X3 = Mat<1, 3, double, MatStoreType::ROW_MAJOR>;
    using PitchAngle = Angle<double, -90., 90., AngleFlags::Clamped>;
    using YawAngle = Angle<double, -180., 180., AngleFlags::Normalized>;
    using RollAngle = Angle<double, -180., 180., AngleFlags::Normalized>;

    using ViewAngles = omath::ViewAngles<PitchAngle, YawAngle, RollAngle>;
} // namespace omath::unreal_engine
