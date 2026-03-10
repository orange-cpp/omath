//
// Created by orange on 08.03.2026.
//
#include <gtest/gtest.h>
#include <lua.hpp>
#include <omath/lua/lua.hpp>

class LuaColor : public ::testing::Test
{
protected:
    lua_State* L = nullptr;

    void SetUp() override
    {
        L = luaL_newstate();
        luaL_openlibs(L);
        omath::lua::LuaInterpreter::register_lib(L);
        if (luaL_dofile(L, LUA_SCRIPTS_DIR "/color_tests.lua") != LUA_OK)
            FAIL() << lua_tostring(L, -1);
    }

    void TearDown() override { lua_close(L); }

    void check(const char* func_name)
    {
        lua_getglobal(L, func_name);
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            FAIL() << lua_tostring(L, -1);
            lua_pop(L, 1);
        }
    }
};

TEST_F(LuaColor, Constructor_float)        { check("Color_Constructor_float"); }
TEST_F(LuaColor, Constructor_default)      { check("Color_Constructor_default"); }
TEST_F(LuaColor, Constructor_clamping)     { check("Color_Constructor_clamping"); }
TEST_F(LuaColor, from_rgba)                { check("Color_from_rgba"); }
TEST_F(LuaColor, from_hsv_components)      { check("Color_from_hsv_components"); }
TEST_F(LuaColor, from_hsv_struct)          { check("Color_from_hsv_struct"); }
TEST_F(LuaColor, red)                      { check("Color_red"); }
TEST_F(LuaColor, green)                    { check("Color_green"); }
TEST_F(LuaColor, blue)                     { check("Color_blue"); }
TEST_F(LuaColor, to_hsv)                   { check("Color_to_hsv"); }
TEST_F(LuaColor, set_hue)                  { check("Color_set_hue"); }
TEST_F(LuaColor, set_saturation)           { check("Color_set_saturation"); }
TEST_F(LuaColor, set_value)                { check("Color_set_value"); }
TEST_F(LuaColor, blend)                    { check("Color_blend"); }
TEST_F(LuaColor, blend_clamped_ratio)      { check("Color_blend_clamped_ratio"); }
TEST_F(LuaColor, to_string)                { check("Color_to_string"); }
TEST_F(LuaColor, Hsv_fields)               { check("Hsv_fields"); }
