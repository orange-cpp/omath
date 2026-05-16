//
// Created by orange on 07.03.2026.
//
#include <gtest/gtest.h>
#include <lua.hpp>
#include <omath/lua/lua.hpp>

class LuaMat : public ::testing::Test
{
protected:
    lua_State* L = nullptr;

    void SetUp() override
    {
        L = luaL_newstate();
        luaL_openlibs(L);
        omath::lua::LuaInterpreter::register_lib(L);
        if (luaL_dofile(L, LUA_SCRIPTS_DIR "/mat_tests.lua") != LUA_OK)
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

TEST_F(LuaMat, Constructor_default)
{
    check("Mat4_Constructor_default");
}
TEST_F(LuaMat, FromRows)
{
    check("Mat4_FromRows");
}
TEST_F(LuaMat, SetAt)
{
    check("Mat4_SetAt");
}
TEST_F(LuaMat, SetAndClear)
{
    check("Mat4_SetAndClear");
}
TEST_F(LuaMat, Multiplication_matrix)
{
    check("Mat4_Multiplication_matrix");
}
TEST_F(LuaMat, Multiplication_scalar)
{
    check("Mat4_Multiplication_scalar");
}
TEST_F(LuaMat, Multiplication_scalar_reversed)
{
    check("Mat4_Multiplication_scalar_reversed");
}
TEST_F(LuaMat, Division_scalar)
{
    check("Mat4_Division_scalar");
}
TEST_F(LuaMat, Transposed)
{
    check("Mat4_Transposed");
}
TEST_F(LuaMat, Determinant)
{
    check("Mat4_Determinant");
}
TEST_F(LuaMat, Inverted_success)
{
    check("Mat4_Inverted_success");
}
TEST_F(LuaMat, Inverted_singular)
{
    check("Mat4_Inverted_singular");
}
TEST_F(LuaMat, ToScreenMat)
{
    check("Mat4_ToScreenMat");
}
TEST_F(LuaMat, Translation)
{
    check("Mat4_Translation");
}
TEST_F(LuaMat, Scale)
{
    check("Mat4_Scale");
}
TEST_F(LuaMat, Perspective)
{
    check("Mat4_Perspective");
}
TEST_F(LuaMat, AsTable)
{
    check("Mat4_AsTable");
}
TEST_F(LuaMat, ToString)
{
    check("Mat4_ToString");
}
TEST_F(LuaMat, Mat3_FromRows)
{
    check("Mat3_FromRows");
}
TEST_F(LuaMat, Mat4ColumnMajor_FromRows)
{
    check("Mat4ColumnMajor_FromRows");
}
TEST_F(LuaMat, Mat4ColumnMajor_ToScreenMat)
{
    check("Mat4ColumnMajor_ToScreenMat");
}
TEST_F(LuaMat, Mat4ColumnMajor_Translation)
{
    check("Mat4ColumnMajor_Translation");
}
TEST_F(LuaMat, Mat4d_FromRows)
{
    check("Mat4d_FromRows");
}
