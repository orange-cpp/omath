#ifdef OMATH_ENABLE_LUA
#include "lua_engines_detail.hpp"
#include <omath/engines/cry_engine/camera.hpp>

namespace omath::lua::detail
{
    void register_cry_engine(sol::table& omath_table)
    {
        register_engine<omath::cry_engine::Camera, omath::cry_engine::PitchAngle, omath::cry_engine::ViewAngles>(
                omath_table, "cry");
    }
} // namespace omath::lua::detail
#endif
