//
// Created by Orange on 11/23/2024.
//
#include <complex>
#include <gtest/gtest.h>
#include <omath/Matrix.hpp>
#include <print>
#include <omath/engines/OpenGL.hpp>
#include <omath/engines/Source.hpp>
// #include <glm/glm.hpp>

// #include "glm/ext/matrix_clip_space.hpp"
// #include "glm/ext/matrix_transform.hpp"


TEST(UnitTestOpenGL, Projection)
{

    /*const auto proj_glm = glm::perspective(glm::radians(90.f), 16.f / 9.f, 0.1f, 1000.f);
    // const auto proj_glm2 = glm::perspectiveLH_NO(glm::radians(90.f), 16.f / 9.f, 0.1f, 1000.f);
    // const auto proj_omath = omath::Mat<4, 4, float, omath::MatStoreType::COLUMN_MAJOR>((const float*)&proj_glm);
    // EXPECT_EQ(omath::opengl::PerspectiveProjectionMatrix(90, 16.f / 9.f, 0.1f, 1000.f), proj_omath);


    glm::vec4 ndc_glm2 = proj_glm * glm::vec4(300.f, 0.f, -1000.f, 1.f);
    ndc_glm2 /= ndc_glm2.w;
    const omath::Mat<4, 1, float, omath::MatStoreType::COLUMN_MAJOR> cords_omath =
    {
        {0},
        {0},
        {-0.2f},
        {1}
    };

    //auto ndc_omath = proj_omath * cords_omath;
   // ndc_omath /= ndc_omath.At(3, 0);
    */
}
TEST(UnitTestOpenGL, Projection2)
{
    const auto orient = omath::opengl::ViewMatrix(omath::opengl::kAbsForward, -omath::opengl::kAbsRight, omath::opengl::kAbsUp, {});

    const omath::Mat<4, 1,float, omath::MatStoreType::COLUMN_MAJOR> cords_omath =
    {
        {0}, {0}, {-10}, {1}
    };
    std::cout << (orient * cords_omath).ToString();
}