//
// Created by Vlad on 9/3/2025.
//

export module omath.unreal_engine.camera;
export import omath.camera;
import omath.unreal_engine.constants;
import omath.unreal_engine.camera_trait;


export namespace omath::unreal_engine
{
    using Camera = projection::Camera<Mat4X4, ViewAngles, CameraTrait>;
}