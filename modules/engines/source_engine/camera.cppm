//
// Created by Orange on 12/4/2024.
//
export module omath.source_engine.camera;

export import omath.projection.camera;
export import omath.source_engine.constants;
export import omath.source_engine.camera_trait;

namespace omath::source_engine
{
    using Camera =  projection::Camera<Mat4X4, ViewAngles, CameraTrait>;
} // namespace omath::source_engine