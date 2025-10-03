//
// Created by Orange on 12/23/2024.
//
export module omath.opengl_egnine.camera;
export import omath.projection.camera;
export import omath.opengl_engine.constants;
export import omath.opengl_engine.camera_trait;

namespace omath::opengl_engine
{
    using Camera =  projection::Camera<Mat4X4, ViewAngles, CameraTrait>;
} // namespace omath::opengl_engine