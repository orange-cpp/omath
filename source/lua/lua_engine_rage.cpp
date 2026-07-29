#ifdef OMATH_ENABLE_LUA
#include "lua_engines_detail.hpp"
#include <omath/engines/rage_engine/camera.hpp>

namespace omath::lua::detail
{
    void register_rage_engine(sol::table& omath_table)
    {
        register_engine<omath::rage_engine::Camera, omath::rage_engine::PitchAngle, omath::rage_engine::ViewAngles>(
                omath_table, "rage");
    }
} // namespace omath::lua::detail
#endif
