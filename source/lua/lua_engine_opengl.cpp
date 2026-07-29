#ifdef OMATH_ENABLE_LUA
#include "lua_engines_detail.hpp"
#include <omath/engines/opengl_engine/camera.hpp>

namespace omath::lua::detail
{
    void register_opengl_engine(sol::table& omath_table)
    {
        register_engine<omath::opengl_engine::Camera, omath::opengl_engine::PitchAngle,
                        omath::opengl_engine::ViewAngles>(omath_table, "opengl");
    }
} // namespace omath::lua::detail
#endif
