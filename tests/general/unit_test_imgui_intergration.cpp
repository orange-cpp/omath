//
// Created by Vlad on 10/23/2025.
//
#ifdef OMATH_IMGUI_INTEGRATION
#include <omath/omath.hpp>
#include <gtest/gtest.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>



using namespace omath;

TEST(unit_test_imgui_intergration, Vector2ToImVec2)
{
    constexpr Vector2 omath_vector_2d = {1.f, 2.f};
    constexpr ImVec2 imgui_vector_2d = {1, 2.f};

    constexpr auto converted = omath_vector_2d.to_im_vec2();
    EXPECT_NEAR(converted.x, imgui_vector_2d.x, 1.e-5f);
    EXPECT_NEAR(converted.y, imgui_vector_2d.y, 1.e-5f);
}

TEST(unit_test_imgui_intergration, Vector4ToImVec4)
{
    constexpr Vector4 omath_vector_2d = {1.f, 2.f, 3.f, 4.f};
    constexpr ImVec4 imgui_vector_4d = {1, 2.f, 3.f, 4.f};

    constexpr auto converted = omath_vector_2d.to_im_vec4();

    EXPECT_NEAR(converted.x, imgui_vector_4d.x, 1.e-5f);
    EXPECT_NEAR(converted.y, imgui_vector_4d.y, 1.e-5f);
    EXPECT_NEAR(converted.z, imgui_vector_4d.z, 1.e-5f);
    EXPECT_NEAR(converted.w, imgui_vector_4d.w, 1.e-5f);
}


#endif
