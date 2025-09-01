//
// Created by Vlad on 9/1/2025.
//

export module omath.iw_engine.camera;
import omath.camera;
import omath.iw_engine.constants;
import omath.iw_engine.camera_trait;
export namespace omath::iw_engine
{
    using Camera = projection::Camera<Mat4X4, ViewAngles, CameraTrait>;
}