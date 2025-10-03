//
// Created by Orange on 12/4/2024.
//
export module omath.source_engine.constants;
export import omath.linear_algebra.vector3;
export import omath.linear_algebra.mat;
export import omath.trigonometry.angle;
export import omath.trigonometry.view_angles;

export namespace omath::source_engine
{
    constexpr Vector3<float> k_abs_up = {0, 0, 1};
    constexpr Vector3<float> k_abs_right = {0, -1, 0};
    constexpr Vector3<float> k_abs_forward = {1, 0, 0};

    using Mat4X4 = Mat<4, 4, float, MatStoreType::ROW_MAJOR>;
    using Mat3X3 = Mat<4, 4, float, MatStoreType::ROW_MAJOR>;
    using Mat1X3 = Mat<1, 3, float, MatStoreType::ROW_MAJOR>;
    using PitchAngle = Angle<float, -89.f, 89.f, AngleFlags::Clamped>;
    using YawAngle = Angle<float, -180.f, 180.f, AngleFlags::Normalized>;
    using RollAngle = Angle<float, -180.f, 180.f, AngleFlags::Normalized>;

    using ViewAngles = omath::ViewAngles<PitchAngle, YawAngle, RollAngle>;
} // namespace omath::source_engine
