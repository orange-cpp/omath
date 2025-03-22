//
// Created by Vlad on 3/22/2025.
//

#pragma once

#include <omath/vector3.hpp>
#include <omath/mat.hpp>
#include <omath/angle.hpp>
#include <omath/view_angles.hpp>

namespace omath::unity_engine
{
    constexpr Vector3<float> kAbsUp = {0, 1, 0};
    constexpr Vector3<float> kAbsRight = {1, 0, 0};
    constexpr Vector3<float> kAbsForward = {0, 0, 1};

    using Mat4x4 = Mat<4, 4, float, MatStoreType::ROW_MAJOR>;
    using Mat3x3 = Mat<4, 4, float, MatStoreType::ROW_MAJOR>;
    using Mat1x3 = Mat<1, 3, float, MatStoreType::ROW_MAJOR>;
    using PitchAngle = Angle<float, -89.f, 89.f, AngleFlags::Clamped>;
    using YawAngle = Angle<float, -180.f, 180.f, AngleFlags::Normalized>;
    using RollAngle = Angle<float, -180.f, 180.f, AngleFlags::Normalized>;

    using ViewAngles = omath::ViewAngles<PitchAngle, YawAngle, RollAngle>;
} // namespace omath::source
