//
// Created by orange on 07.03.2026.
//
#include <gtest/gtest.h>
#include <lua.hpp>
#include <omath/lua/lua.hpp>

class LuaHud : public ::testing::Test
{
protected:
    lua_State* L = nullptr;

    void SetUp() override
    {
        L = luaL_newstate();
        luaL_openlibs(L);
        omath::lua::LuaInterpreter::register_lib(L);
        if (luaL_dofile(L, LUA_SCRIPTS_DIR "/hud_tests.lua") != LUA_OK)
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

TEST_F(LuaHud, CanvasBox_default_ratio)          { check("Hud_CanvasBox_default_ratio"); }
TEST_F(LuaHud, CanvasBox_custom_ratio)           { check("Hud_CanvasBox_custom_ratio"); }
TEST_F(LuaHud, CanvasBox_as_table)               { check("Hud_CanvasBox_as_table"); }
TEST_F(LuaHud, Renderer_callbacks)               { check("Hud_Renderer_callbacks"); }
TEST_F(LuaHud, Renderer_polyline_table)          { check("Hud_Renderer_polyline_table"); }
TEST_F(LuaHud, Renderer_circle_defaults)         { check("Hud_Renderer_circle_defaults"); }
TEST_F(LuaHud, EntityOverlay_add_2d_box)         { check("Hud_EntityOverlay_add_2d_box"); }
TEST_F(LuaHud, EntityOverlay_add_cornered_2d_box) { check("Hud_EntityOverlay_add_cornered_2d_box"); }
TEST_F(LuaHud, EntityOverlay_add_dashed_box)     { check("Hud_EntityOverlay_add_dashed_box"); }
TEST_F(LuaHud, EntityOverlay_add_bar)            { check("Hud_EntityOverlay_add_bar"); }
TEST_F(LuaHud, EntityOverlay_add_dashed_bar)     { check("Hud_EntityOverlay_add_dashed_bar"); }
TEST_F(LuaHud, EntityOverlay_add_label)          { check("Hud_EntityOverlay_add_label"); }
TEST_F(LuaHud, EntityOverlay_add_outlined_label) { check("Hud_EntityOverlay_add_outlined_label"); }
TEST_F(LuaHud, EntityOverlay_add_centered_label) { check("Hud_EntityOverlay_add_centered_label"); }
TEST_F(LuaHud, EntityOverlay_add_spaces)         { check("Hud_EntityOverlay_add_spaces"); }
TEST_F(LuaHud, EntityOverlay_add_progress_ring)  { check("Hud_EntityOverlay_add_progress_ring"); }
TEST_F(LuaHud, EntityOverlay_add_progress_ring_defaults) { check("Hud_EntityOverlay_add_progress_ring_defaults"); }
TEST_F(LuaHud, EntityOverlay_add_icon)           { check("Hud_EntityOverlay_add_icon"); }
TEST_F(LuaHud, EntityOverlay_add_snap_line)      { check("Hud_EntityOverlay_add_snap_line"); }
TEST_F(LuaHud, EntityOverlay_add_skeleton)       { check("Hud_EntityOverlay_add_skeleton"); }
TEST_F(LuaHud, EntityOverlay_add_scan_marker)    { check("Hud_EntityOverlay_add_scan_marker"); }
TEST_F(LuaHud, EntityOverlay_add_aim_dot)        { check("Hud_EntityOverlay_add_aim_dot"); }
TEST_F(LuaHud, EntityOverlay_add_projectile_aim_circle) { check("Hud_EntityOverlay_add_projectile_aim_circle"); }
TEST_F(LuaHud, EntityOverlay_add_projectile_aim_square_default) { check("Hud_EntityOverlay_add_projectile_aim_square_default"); }
