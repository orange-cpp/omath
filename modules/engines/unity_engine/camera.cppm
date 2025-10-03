//
// Created by Vlad on 3/22/2025.
//


export module omath.unitiy_engine.camera;
export import omath.projection.camera;
export import omath.unity_egnine.constants;
export import omath.unity_engine.camera_trait;


namespace omath::unity_engine
{
    using Camera = projection::Camera<Mat4X4, ViewAngles, CameraTrait>;
} // namespace omath::unity_engine