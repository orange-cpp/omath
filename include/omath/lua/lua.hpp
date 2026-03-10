//
// Created by orange on 07.03.2026.
//
#pragma once
#ifdef OMATH_ENABLE_LUA
// ReSharper disable once CppInconsistentNaming
struct lua_State;

namespace omath::lua
{
    void register_lib(lua_State* lua_state);
}
namespace omath::lua::detail
{
    void register_vec2(sol::table& omath_table);
    void register_vec3(sol::table& omath_table);
    void register_vec4(sol::table& omath_table);
    void register_color(sol::table& omath_table);
    void register_shared_types(sol::table& omath_table);
    void register_engines(sol::table& omath_table);
}
#endif