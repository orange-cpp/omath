//
// Created by Vlad on 9/2/2025.
//

export module omath.source_engine.camera;
import omath.camera;
export import omath.source_engine.constants;
export import omath.source_engine.camera_trait;

export namespace omath::source_engine
{
    using Camera = projection::Camera<Mat4X4, ViewAngles, CameraTrait>;
}