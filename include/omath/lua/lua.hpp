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
#endif