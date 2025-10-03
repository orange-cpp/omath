//
// Created by Vlad on 3/22/2025.
//


export module omath.unity_engine.camera;
export import omath.projection.camera;
export import omath.unity_engine.constants;
export import omath.unity_engine.camera_trait;


export namespace omath::unity_engine
{
    using Camera = projection::Camera<Mat4X4, ViewAngles, CameraTrait>;
} // namespace omath::unity_engine