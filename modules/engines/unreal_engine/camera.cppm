//
// Created by Vlad on 3/22/2025.
//

module;
export module omath.unreal_engine.camera;
export import omath.projection.camera;
export import omath.unreal_engine.constants;
export import omath.unreal_engine.camera_trait;


namespace omath::unreal_engine
{
    using Camera = projection::Camera<Mat4X4, ViewAngles, CameraTrait>;
} // namespace omath::unreal_engine