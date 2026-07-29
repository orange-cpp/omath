#ifdef OMATH_ENABLE_LUA
#include "lua_engines_detail.hpp"
#include <omath/engines/unity_engine/camera.hpp>

namespace omath::lua::detail
{
    void register_unity_engine(sol::table& omath_table)
    {
        register_engine<omath::unity_engine::Camera, omath::unity_engine::PitchAngle,
                        omath::unity_engine::ViewAngles>(omath_table, "unity");
    }
} // namespace omath::lua::detail
#endif
