//
// Created by Vlad on 9/2/2025.
//

export module omath.opengl_engine.constants;

import omath.vector3;
import omath.mat;
import omath.angle;
import omath.view_angles;

export namespace omath::opengl_engine
{
    constexpr Vector3<float> k_abs_up = {0, 1, 0};
    constexpr Vector3<float> k_abs_right = {1, 0, 0};
    constexpr Vector3<float> k_abs_forward = {0, 0, -1};

    using Mat4X4 = Mat<4, 4, float, MatStoreType::COLUMN_MAJOR>;
    using Mat3X3 = Mat<4, 4, float, MatStoreType::COLUMN_MAJOR>;
    using Mat1X3 = Mat<1, 3, float, MatStoreType::COLUMN_MAJOR>;
    using PitchAngle = Angle<float, -90.f, 90.f, AngleFlags::Clamped>;
    using YawAngle = Angle<float, -180.f, 180.f, AngleFlags::Normalized>;
    using RollAngle = Angle<float, -180.f, 180.f, AngleFlags::Normalized>;

    using ViewAngles = omath::ViewAngles<PitchAngle, YawAngle, RollAngle>;
} // namespace omath::opengl_engine