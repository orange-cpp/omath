//
// Created by orange on 10.03.2026.
//
#include <gtest/gtest.h>
#include <lua.hpp>
#include <omath/lua/lua.hpp>

class LuaTriangle : public ::testing::Test
{
protected:
    lua_State* L = nullptr;

    void SetUp() override
    {
        L = luaL_newstate();
        luaL_openlibs(L);
        omath::lua::LuaInterpreter::register_lib(L);
        if (luaL_dofile(L, LUA_SCRIPTS_DIR "/triangle_tests.lua") != LUA_OK)
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

TEST_F(LuaTriangle, Constructor_default)    { check("Triangle_Constructor_default"); }
TEST_F(LuaTriangle, Constructor_vertices)   { check("Triangle_Constructor_vertices"); }
TEST_F(LuaTriangle, Vertex_mutation)        { check("Triangle_Vertex_mutation"); }
TEST_F(LuaTriangle, SideALength)            { check("Triangle_SideALength"); }
TEST_F(LuaTriangle, SideBLength)            { check("Triangle_SideBLength"); }
TEST_F(LuaTriangle, Hypot)                  { check("Triangle_Hypot"); }
TEST_F(LuaTriangle, SideAVector)            { check("Triangle_SideAVector"); }
TEST_F(LuaTriangle, SideBVector)            { check("Triangle_SideBVector"); }
TEST_F(LuaTriangle, IsRectangular_true)     { check("Triangle_IsRectangular_true"); }
TEST_F(LuaTriangle, IsRectangular_false)    { check("Triangle_IsRectangular_false"); }
TEST_F(LuaTriangle, MidPoint)               { check("Triangle_MidPoint"); }
TEST_F(LuaTriangle, CalculateNormal)        { check("Triangle_CalculateNormal"); }
TEST_F(LuaTriangle, ToString)               { check("Triangle_ToString"); }
