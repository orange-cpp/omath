//
// Created by Vlad on 3/17/2025.
//
export module omath.iw_engine.camera;
export import omath.projection.camera;
export import omath.iw_engine.constants;
export import omath.iw_engine.camera_trait;

namespace omath::iw_engine
{
    using Camera =  projection::Camera<Mat4X4, ViewAngles, CameraTrait>;
} // namespace omath::iw_engine