//
// Created by orange on 07.03.2026.
//
#include <gtest/gtest.h>
#include <lua.hpp>
#include <omath/lua/lua.hpp>

class LuaQuaternion : public ::testing::Test
{
protected:
    lua_State* L = nullptr;

    void SetUp() override
    {
        L = luaL_newstate();
        luaL_openlibs(L);
        omath::lua::LuaInterpreter::register_lib(L);
        if (luaL_dofile(L, LUA_SCRIPTS_DIR "/quaternion_tests.lua") != LUA_OK)
            FAIL() << lua_tostring(L, -1);
    }

    void TearDown() override
    {
        lua_close(L);
    }

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

TEST_F(LuaQuaternion, Constructor_default)
{
    check("Quaternion_Constructor_default");
}
TEST_F(LuaQuaternion, Constructor_xyzw)
{
    check("Quaternion_Constructor_xyzw");
}
TEST_F(LuaQuaternion, Field_mutation)
{
    check("Quaternion_Field_mutation");
}
TEST_F(LuaQuaternion, FromAxisAngle_zero_angle)
{
    check("Quaternion_FromAxisAngle_zero_angle");
}
TEST_F(LuaQuaternion, Addition)
{
    check("Quaternion_Addition");
}
TEST_F(LuaQuaternion, Multiplication_scalar)
{
    check("Quaternion_Multiplication_scalar");
}
TEST_F(LuaQuaternion, Multiplication_scalar_reversed)
{
    check("Quaternion_Multiplication_scalar_reversed");
}
TEST_F(LuaQuaternion, Multiplication_quaternion)
{
    check("Quaternion_Multiplication_quaternion");
}
TEST_F(LuaQuaternion, UnaryMinus)
{
    check("Quaternion_UnaryMinus");
}
TEST_F(LuaQuaternion, EqualTo_true)
{
    check("Quaternion_EqualTo_true");
}
TEST_F(LuaQuaternion, EqualTo_false)
{
    check("Quaternion_EqualTo_false");
}
TEST_F(LuaQuaternion, ToString)
{
    check("Quaternion_ToString");
}
TEST_F(LuaQuaternion, Conjugate)
{
    check("Quaternion_Conjugate");
}
TEST_F(LuaQuaternion, Dot)
{
    check("Quaternion_Dot");
}
TEST_F(LuaQuaternion, Length)
{
    check("Quaternion_Length");
}
TEST_F(LuaQuaternion, LengthSqr)
{
    check("Quaternion_LengthSqr");
}
TEST_F(LuaQuaternion, Normalized)
{
    check("Quaternion_Normalized");
}
TEST_F(LuaQuaternion, Inverse)
{
    check("Quaternion_Inverse");
}
TEST_F(LuaQuaternion, Rotate)
{
    check("Quaternion_Rotate");
}
TEST_F(LuaQuaternion, ToRotationMatrix3)
{
    check("Quaternion_ToRotationMatrix3");
}
TEST_F(LuaQuaternion, ToRotationMatrix4)
{
    check("Quaternion_ToRotationMatrix4");
}
TEST_F(LuaQuaternion, AsTable)
{
    check("Quaternion_AsTable");
}
