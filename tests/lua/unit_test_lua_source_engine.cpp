//
// Created by orange on 07.03.2026.
//
#include <gtest/gtest.h>
#include <lua.hpp>
#include <omath/lua/lua.hpp>

class LuaSourceEngine : public ::testing::Test
{
protected:
    lua_State* L = nullptr;

    void SetUp() override
    {
        L = luaL_newstate();
        luaL_openlibs(L);
        omath::lua::register_lib(L);
        if (luaL_dofile(L, LUA_SCRIPTS_DIR "/source_engine_tests.lua") != LUA_OK)
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

// PitchAngle
TEST_F(LuaSourceEngine, PitchAngle_from_degrees)        { check("Source_PitchAngle_from_degrees"); }
TEST_F(LuaSourceEngine, PitchAngle_clamping_max)        { check("Source_PitchAngle_clamping_max"); }
TEST_F(LuaSourceEngine, PitchAngle_clamping_min)        { check("Source_PitchAngle_clamping_min"); }
TEST_F(LuaSourceEngine, PitchAngle_from_radians)        { check("Source_PitchAngle_from_radians"); }
TEST_F(LuaSourceEngine, PitchAngle_as_radians)          { check("Source_PitchAngle_as_radians"); }
TEST_F(LuaSourceEngine, PitchAngle_sin)                 { check("Source_PitchAngle_sin"); }
TEST_F(LuaSourceEngine, PitchAngle_cos)                 { check("Source_PitchAngle_cos"); }
TEST_F(LuaSourceEngine, PitchAngle_tan)                 { check("Source_PitchAngle_tan"); }
TEST_F(LuaSourceEngine, PitchAngle_addition)            { check("Source_PitchAngle_addition"); }
TEST_F(LuaSourceEngine, PitchAngle_addition_clamped)    { check("Source_PitchAngle_addition_clamped"); }
TEST_F(LuaSourceEngine, PitchAngle_subtraction)         { check("Source_PitchAngle_subtraction"); }
TEST_F(LuaSourceEngine, PitchAngle_unary_minus)         { check("Source_PitchAngle_unary_minus"); }
TEST_F(LuaSourceEngine, PitchAngle_equal_to)            { check("Source_PitchAngle_equal_to"); }
TEST_F(LuaSourceEngine, PitchAngle_to_string)           { check("Source_PitchAngle_to_string"); }

// YawAngle
TEST_F(LuaSourceEngine, YawAngle_from_degrees)          { check("Source_YawAngle_from_degrees"); }
TEST_F(LuaSourceEngine, YawAngle_normalization)         { check("Source_YawAngle_normalization"); }

// RollAngle
TEST_F(LuaSourceEngine, RollAngle_from_degrees)         { check("Source_RollAngle_from_degrees"); }

// FieldOfView
TEST_F(LuaSourceEngine, FieldOfView_from_degrees)       { check("Source_FieldOfView_from_degrees"); }
TEST_F(LuaSourceEngine, FieldOfView_clamping)           { check("Source_FieldOfView_clamping"); }

// ViewAngles
TEST_F(LuaSourceEngine, ViewAngles_new)                 { check("Source_ViewAngles_new"); }
TEST_F(LuaSourceEngine, ViewAngles_field_mutation)      { check("Source_ViewAngles_field_mutation"); }

// Camera
TEST_F(LuaSourceEngine, Camera_constructor)             { check("Source_Camera_constructor"); }
TEST_F(LuaSourceEngine, Camera_get_set_origin)          { check("Source_Camera_get_set_origin"); }
TEST_F(LuaSourceEngine, Camera_get_set_near_plane)      { check("Source_Camera_get_set_near_plane"); }
TEST_F(LuaSourceEngine, Camera_get_set_far_plane)       { check("Source_Camera_get_set_far_plane"); }
TEST_F(LuaSourceEngine, Camera_get_set_fov)             { check("Source_Camera_get_set_fov"); }
TEST_F(LuaSourceEngine, Camera_get_set_view_angles)     { check("Source_Camera_get_set_view_angles"); }
TEST_F(LuaSourceEngine, Camera_look_at)                 { check("Source_Camera_look_at"); }
TEST_F(LuaSourceEngine, Camera_get_forward)             { check("Source_Camera_get_forward"); }
TEST_F(LuaSourceEngine, Camera_get_right)               { check("Source_Camera_get_right"); }
TEST_F(LuaSourceEngine, Camera_get_up)                  { check("Source_Camera_get_up"); }
TEST_F(LuaSourceEngine, Camera_world_to_screen_success) { check("Source_Camera_world_to_screen_success"); }
TEST_F(LuaSourceEngine, Camera_world_to_screen_error)   { check("Source_Camera_world_to_screen_error"); }
TEST_F(LuaSourceEngine, Camera_screen_to_world)         { check("Source_Camera_screen_to_world"); }
