#ifdef OMATH_ENABLE_LUA
#include "lua_engines_detail.hpp"
#include <omath/engines/iw_engine/camera.hpp>

namespace omath::lua::detail
{
    void register_iw_engine(sol::table& omath_table)
    {
        register_engine<omath::iw_engine::Camera, omath::iw_engine::PitchAngle, omath::iw_engine::ViewAngles>(
                omath_table, "iw");
    }
} // namespace omath::lua::detail
#endif
