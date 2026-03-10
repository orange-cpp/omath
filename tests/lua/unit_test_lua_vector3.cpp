//
// Created by orange on 07.03.2026.
//
#include <gtest/gtest.h>
#include <lua.hpp>
#include <omath/lua/lua.hpp>

class LuaVec3 : public ::testing::Test
{
protected:
    lua_State* L = nullptr;

    void SetUp() override
    {
        L = luaL_newstate();
        luaL_openlibs(L);
        omath::lua::LuaInterpreter::register_lib(L);
        if (luaL_dofile(L, LUA_SCRIPTS_DIR "/vec3_tests.lua") != LUA_OK)
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

TEST_F(LuaVec3, Constructor_default)            { check("Vec3_Constructor_default"); }
TEST_F(LuaVec3, Constructor_xyz)                { check("Vec3_Constructor_xyz"); }
TEST_F(LuaVec3, Field_mutation)                 { check("Vec3_Field_mutation"); }
TEST_F(LuaVec3, Addition)                       { check("Vec3_Addition"); }
TEST_F(LuaVec3, Subtraction)                    { check("Vec3_Subtraction"); }
TEST_F(LuaVec3, UnaryMinus)                     { check("Vec3_UnaryMinus"); }
TEST_F(LuaVec3, Multiplication_scalar)          { check("Vec3_Multiplication_scalar"); }
TEST_F(LuaVec3, Multiplication_scalar_reversed) { check("Vec3_Multiplication_scalar_reversed"); }
TEST_F(LuaVec3, Multiplication_vec)             { check("Vec3_Multiplication_vec"); }
TEST_F(LuaVec3, Division_scalar)                { check("Vec3_Division_scalar"); }
TEST_F(LuaVec3, Division_vec)                   { check("Vec3_Division_vec"); }
TEST_F(LuaVec3, EqualTo_true)                   { check("Vec3_EqualTo_true"); }
TEST_F(LuaVec3, EqualTo_false)                  { check("Vec3_EqualTo_false"); }
TEST_F(LuaVec3, LessThan)                       { check("Vec3_LessThan"); }
TEST_F(LuaVec3, LessThanOrEqual)                { check("Vec3_LessThanOrEqual"); }
TEST_F(LuaVec3, ToString)                       { check("Vec3_ToString"); }
TEST_F(LuaVec3, Length)                         { check("Vec3_Length"); }
TEST_F(LuaVec3, Length2d)                       { check("Vec3_Length2d"); }
TEST_F(LuaVec3, LengthSqr)                      { check("Vec3_LengthSqr"); }
TEST_F(LuaVec3, Normalized)                     { check("Vec3_Normalized"); }
TEST_F(LuaVec3, Dot_perpendicular)              { check("Vec3_Dot_perpendicular"); }
TEST_F(LuaVec3, Dot_parallel)                   { check("Vec3_Dot_parallel"); }
TEST_F(LuaVec3, Cross)                          { check("Vec3_Cross"); }
TEST_F(LuaVec3, DistanceTo)                     { check("Vec3_DistanceTo"); }
TEST_F(LuaVec3, DistanceToSqr)                  { check("Vec3_DistanceToSqr"); }
TEST_F(LuaVec3, Sum)                            { check("Vec3_Sum"); }
TEST_F(LuaVec3, Sum2d)                          { check("Vec3_Sum2d"); }
TEST_F(LuaVec3, Abs)                            { check("Vec3_Abs"); }
TEST_F(LuaVec3, PointToSameDirection_true)      { check("Vec3_PointToSameDirection_true"); }
TEST_F(LuaVec3, PointToSameDirection_false)     { check("Vec3_PointToSameDirection_false"); }
TEST_F(LuaVec3, IsPerpendicular_true)           { check("Vec3_IsPerpendicular_true"); }
TEST_F(LuaVec3, IsPerpendicular_false)          { check("Vec3_IsPerpendicular_false"); }
TEST_F(LuaVec3, AngleBetween_90deg)             { check("Vec3_AngleBetween_90deg"); }
TEST_F(LuaVec3, AngleBetween_zero_vector_error) { check("Vec3_AngleBetween_zero_vector_error"); }
TEST_F(LuaVec3, AsTable)                        { check("Vec3_AsTable"); }
