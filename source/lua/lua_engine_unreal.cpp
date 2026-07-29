#ifdef OMATH_ENABLE_LUA
#include "lua_engines_detail.hpp"
#include <omath/engines/unreal_engine/camera.hpp>

namespace omath::lua::detail
{
    void register_unreal_engine(sol::table& omath_table)
    {
        register_engine<omath::unreal_engine::Camera, omath::unreal_engine::PitchAngle,
                        omath::unreal_engine::ViewAngles, double>(omath_table, "unreal");
    }
} // namespace omath::lua::detail
#endif
