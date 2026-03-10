//
// Created by orange on 07.03.2026.
//
#pragma once
#ifdef OMATH_ENABLE_LUA
#include <sol/forward.hpp>
namespace omath::lua
{
    class LuaInterpreter final
    {
    public:
        static void register_lib(lua_State* lua_state);

    private:
        static void register_vec2(sol::table& omath_table);
        static void register_vec3(sol::table& omath_table);
        static void register_vec4(sol::table& omath_table);
        static void register_color(sol::table& omath_table);
        static void register_shared_types(sol::table& omath_table);
        static void register_engines(sol::table& omath_table);
    };
}
#endif