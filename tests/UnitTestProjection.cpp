//
// Created by Vlad on 27.08.2024.
//
#include <complex>
#include <gtest/gtest.h>
#include <omath/Matrix.h>
#include <omath/projection/Camera.h>


#include <print>

TEST(UnitTestProjection, Projection)
{
    const omath::projection::Camera camera({0.f, 0.f, 0.f}, {0, 0.f, 0.f} , {1920.f, 1080.f}, 110.f, 0.1f, 100.f);
    const auto proj = camera.WorldToScreen({10, 0, 20}).value();

   std::println("{} {} {}", proj.x, proj.y, proj.z);

}