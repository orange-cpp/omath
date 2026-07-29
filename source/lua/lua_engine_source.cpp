#ifdef OMATH_ENABLE_LUA
#include "lua_engines_detail.hpp"
#include <omath/engines/source_engine/camera.hpp>

namespace omath::lua::detail
{
    void register_source_engine(sol::table& omath_table)
    {
        register_engine<omath::source_engine::Camera, omath::source_engine::PitchAngle,
                        omath::source_engine::ViewAngles>(omath_table, "source");
    }
} // namespace omath::lua::detail
#endif
