#ifdef OMATH_ENABLE_LUA
#include "lua_engines_detail.hpp"
#include <omath/engines/frostbite_engine/camera.hpp>

namespace omath::lua::detail
{
    void register_frostbite_engine(sol::table& omath_table)
    {
        register_engine<omath::frostbite_engine::Camera, omath::frostbite_engine::PitchAngle,
                        omath::frostbite_engine::ViewAngles>(omath_table, "frostbite");
    }
} // namespace omath::lua::detail
#endif
