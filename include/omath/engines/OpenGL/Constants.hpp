//
// Created by Orange on 12/23/2024.
//
#pragma once

#include <omath/Vector3.hpp>
#include <omath/Mat.hpp>
#include <omath/Angle.hpp>
#include <omath/ViewAngles.hpp>

namespace omath::opengl
{
    constexpr Vector3 kAbsUp = {0, 1, 0};
    constexpr Vector3 kAbsRight = {1, 0, 0};
    constexpr Vector3 kAbsForward = {0, 0, -1};

    using Mat4x4 = Mat<4, 4, float, MatStoreType::COLUMN_MAJOR>;
    using Mat3x3 = Mat<4, 4, float, MatStoreType::COLUMN_MAJOR>;
    using Mat1x3 = Mat<1, 3, float, MatStoreType::COLUMN_MAJOR>;
    using PitchAngle = Angle<float, 0.f, 180.f, AngleFlags::Clamped>;
    using YawAngle = Angle<float, 0.f, 360.f, AngleFlags::Normalized>;
    using RollAngle = Angle<float, 0.f, 360.f, AngleFlags::Normalized>;

    using ViewAngles = omath::ViewAngles<PitchAngle, YawAngle, RollAngle>;
}