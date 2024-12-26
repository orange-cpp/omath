//
// Created by Orange on 12/4/2024.
//
#pragma once

#include <omath/Vector3.hpp>
#include <omath/Mat.hpp>
#include <omath/Angle.hpp>
#include <omath/ViewAngles.hpp>
namespace omath::source
{
    constexpr Vector3 kAbsUp = {0, 0, 1};
    constexpr Vector3 kAbsRight = {0, -1, 0};
    constexpr Vector3 kAbsForward = {1, 0, 0};

    using Mat4x4 = Mat<4, 4, float, MatStoreType::ROW_MAJOR>;
    using Mat3x3 = Mat<4, 4, float, MatStoreType::ROW_MAJOR>;
    using Mat1x3 = Mat<1, 3, float, MatStoreType::ROW_MAJOR>;
    using PitchAngle = Angle<float, -89.f, 89.f, AngleFlags::Clamped>;
    using YawAngle = Angle<float, -180.f, 180.f, AngleFlags::Normalized>;
    using RollAngle = Angle<float, -180.f, 180.f, AngleFlags::Normalized>;

    using ViewAngles = omath::ViewAngles<PitchAngle, YawAngle, RollAngle>;
} // namespace omath::source
