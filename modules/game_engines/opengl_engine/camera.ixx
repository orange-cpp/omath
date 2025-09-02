//
// Created by Vlad on 9/2/2025.
//

export module omath.opengl_engine.camera;

export import omath.camera;
import omath.opengl_engine.constants;
import omath.opengl_engine.camera_trait;

export namespace omath::opengl_engine
{
    using Camera = projection::Camera<Mat4X4, ViewAngles, CameraTrait>;
}