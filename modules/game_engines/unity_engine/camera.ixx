//
// Created by Vlad on 9/3/2025.
//

export module omath.unity_engine.camera;
import omath.camera;
import omath.unity_engine.constants;
import omath.unity_engine.camera_trait;


export namespace omath::unity_engine
{
    using Camera = projection::Camera<Mat4X4, ViewAngles, CameraTrait>;
}