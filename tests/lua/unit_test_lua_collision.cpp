//
// Created by orange on 07.03.2026.
//
#include <gtest/gtest.h>
#include <lua.hpp>
#include <omath/lua/lua.hpp>

class LuaCollision : public ::testing::Test
{
protected:
    lua_State* L = nullptr;

    void SetUp() override
    {
        L = luaL_newstate();
        luaL_openlibs(L);
        omath::lua::LuaInterpreter::register_lib(L);
        if (luaL_dofile(L, LUA_SCRIPTS_DIR "/collision_tests.lua") != LUA_OK)
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

TEST_F(LuaCollision, Aabb_constructor_and_fields)      { check("Collision_Aabb_constructor_and_fields"); }
TEST_F(LuaCollision, Aabb_center_extents)              { check("Collision_Aabb_center_extents"); }
TEST_F(LuaCollision, Aabb_top_bottom_default_axis)     { check("Collision_Aabb_top_bottom_default_axis"); }
TEST_F(LuaCollision, Aabb_top_bottom_explicit_axis)    { check("Collision_Aabb_top_bottom_explicit_axis"); }
TEST_F(LuaCollision, Aabb_is_collide)                  { check("Collision_Aabb_is_collide"); }
TEST_F(LuaCollision, Aabb_as_table)                    { check("Collision_Aabb_as_table"); }
TEST_F(LuaCollision, Obb_constructor_and_vertices)     { check("Collision_Obb_constructor_and_vertices"); }
TEST_F(LuaCollision, Ray_constructor_and_direction)    { check("Collision_Ray_constructor_and_direction"); }
TEST_F(LuaCollision, LineTracer_triangle_hit)          { check("Collision_LineTracer_triangle_hit"); }
TEST_F(LuaCollision, LineTracer_triangle_miss)         { check("Collision_LineTracer_triangle_miss"); }
TEST_F(LuaCollision, LineTracer_aabb_hit)              { check("Collision_LineTracer_aabb_hit"); }
TEST_F(LuaCollision, LineTracer_aabb_miss)             { check("Collision_LineTracer_aabb_miss"); }
TEST_F(LuaCollision, LineTracer_obb_hit)               { check("Collision_LineTracer_obb_hit"); }
TEST_F(LuaCollision, ConvexCollider_vertices_support)  { check("Collision_ConvexCollider_vertices_and_support"); }
TEST_F(LuaCollision, Gjk_support_vertex)               { check("Collision_Gjk_support_vertex"); }
TEST_F(LuaCollision, Gjk_collide_true)                 { check("Collision_Gjk_collide_true"); }
TEST_F(LuaCollision, Gjk_collide_false)                { check("Collision_Gjk_collide_false"); }
TEST_F(LuaCollision, Epa_solve_hit)                    { check("Collision_Epa_solve_hit"); }
TEST_F(LuaCollision, Epa_solve_miss)                   { check("Collision_Epa_solve_miss"); }
