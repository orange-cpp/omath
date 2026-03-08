//
// Created by orange on 07.03.2026.
//
#include <gtest/gtest.h>
#include <lua.hpp>
#include <omath/lua/lua.hpp>

class LuaVec2 : public ::testing::Test
{
protected:
    lua_State* L = nullptr;

    void SetUp() override
    {
        L = luaL_newstate();
        luaL_openlibs(L);
        omath::lua::register_lib(L);
        if (luaL_dofile(L, LUA_SCRIPTS_DIR "/vec2_tests.lua") != LUA_OK)
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

TEST_F(LuaVec2, Constructor_default)            { check("Vec2_Constructor_default"); }
TEST_F(LuaVec2, Constructor_xy)                 { check("Vec2_Constructor_xy"); }
TEST_F(LuaVec2, Field_mutation)                 { check("Vec2_Field_mutation"); }
TEST_F(LuaVec2, Addition)                       { check("Vec2_Addition"); }
TEST_F(LuaVec2, Subtraction)                    { check("Vec2_Subtraction"); }
TEST_F(LuaVec2, UnaryMinus)                     { check("Vec2_UnaryMinus"); }
TEST_F(LuaVec2, Multiplication_scalar)          { check("Vec2_Multiplication_scalar"); }
TEST_F(LuaVec2, Multiplication_scalar_reversed) { check("Vec2_Multiplication_scalar_reversed"); }
TEST_F(LuaVec2, Division_scalar)                { check("Vec2_Division_scalar"); }
TEST_F(LuaVec2, EqualTo_true)                   { check("Vec2_EqualTo_true"); }
TEST_F(LuaVec2, EqualTo_false)                  { check("Vec2_EqualTo_false"); }
TEST_F(LuaVec2, LessThan)                       { check("Vec2_LessThan"); }
TEST_F(LuaVec2, LessThanOrEqual)                { check("Vec2_LessThanOrEqual"); }
TEST_F(LuaVec2, ToString)                       { check("Vec2_ToString"); }
TEST_F(LuaVec2, Length)                         { check("Vec2_Length"); }
TEST_F(LuaVec2, LengthSqr)                      { check("Vec2_LengthSqr"); }
TEST_F(LuaVec2, Normalized)                     { check("Vec2_Normalized"); }
TEST_F(LuaVec2, Dot)                            { check("Vec2_Dot"); }
TEST_F(LuaVec2, DistanceTo)                     { check("Vec2_DistanceTo"); }
TEST_F(LuaVec2, DistanceToSqr)                  { check("Vec2_DistanceToSqr"); }
TEST_F(LuaVec2, Sum)                            { check("Vec2_Sum"); }
TEST_F(LuaVec2, Abs)                            { check("Vec2_Abs"); }
