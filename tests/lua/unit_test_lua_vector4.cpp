//
// Created by orange on 07.03.2026.
//
#include <gtest/gtest.h>
#include <lua.hpp>
#include <omath/lua/lua.hpp>

class LuaVec4 : public ::testing::Test
{
protected:
    lua_State* L = nullptr;

    void SetUp() override
    {
        L = luaL_newstate();
        luaL_openlibs(L);
        omath::lua::register_lib(L);
        if (luaL_dofile(L, LUA_SCRIPTS_DIR "/vec4_tests.lua") != LUA_OK)
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

TEST_F(LuaVec4, Constructor_default)            { check("Vec4_Constructor_default"); }
TEST_F(LuaVec4, Constructor_xyzw)               { check("Vec4_Constructor_xyzw"); }
TEST_F(LuaVec4, Field_mutation)                 { check("Vec4_Field_mutation"); }
TEST_F(LuaVec4, Addition)                       { check("Vec4_Addition"); }
TEST_F(LuaVec4, Subtraction)                    { check("Vec4_Subtraction"); }
TEST_F(LuaVec4, UnaryMinus)                     { check("Vec4_UnaryMinus"); }
TEST_F(LuaVec4, Multiplication_scalar)          { check("Vec4_Multiplication_scalar"); }
TEST_F(LuaVec4, Multiplication_scalar_reversed) { check("Vec4_Multiplication_scalar_reversed"); }
TEST_F(LuaVec4, Multiplication_vec)             { check("Vec4_Multiplication_vec"); }
TEST_F(LuaVec4, Division_scalar)                { check("Vec4_Division_scalar"); }
TEST_F(LuaVec4, Division_vec)                   { check("Vec4_Division_vec"); }
TEST_F(LuaVec4, EqualTo_true)                   { check("Vec4_EqualTo_true"); }
TEST_F(LuaVec4, EqualTo_false)                  { check("Vec4_EqualTo_false"); }
TEST_F(LuaVec4, LessThan)                       { check("Vec4_LessThan"); }
TEST_F(LuaVec4, LessThanOrEqual)                { check("Vec4_LessThanOrEqual"); }
TEST_F(LuaVec4, ToString)                       { check("Vec4_ToString"); }
TEST_F(LuaVec4, Length)                         { check("Vec4_Length"); }
TEST_F(LuaVec4, LengthSqr)                      { check("Vec4_LengthSqr"); }
TEST_F(LuaVec4, Dot)                            { check("Vec4_Dot"); }
TEST_F(LuaVec4, Dot_perpendicular)              { check("Vec4_Dot_perpendicular"); }
TEST_F(LuaVec4, Sum)                            { check("Vec4_Sum"); }
TEST_F(LuaVec4, Abs)                            { check("Vec4_Abs"); }
TEST_F(LuaVec4, Clamp)                          { check("Vec4_Clamp"); }
