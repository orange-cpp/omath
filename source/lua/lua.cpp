//
// Created by orange on 07.03.2026.
//
#ifdef OMATH_ENABLE_LUA
#include "lua.hpp"
#include <sol/sol.hpp>
#include "omath/lua/lua.hpp"

namespace omath::lua
{
    void LuaInterpreter::register_lib(lua_State* lua_state)
    {
        sol::state_view lua(lua_state);

        auto omath_table = lua["omath"].get_or_create<sol::table>();

        register_vec2(omath_table);
        register_vec3(omath_table);
        register_vec4(omath_table);
        register_color(omath_table);
        register_shared_types(omath_table);
        register_engines(omath_table);
    }
} // namespace omath::lua
#endif
