//
// Created by Vlad on 27.08.2024.
//
#include <complex>
#include <gtest/gtest.h>
#include <omath/Matrix.hpp>
#include <omath/engines/Source/Camera.hpp>
#include <omath/projection/Camera.hpp>
#include <print>

TEST(UnitTestProjection, Projection)
{
    auto x = omath::Angle<float, 0.f, 180.f, omath::AngleFlags::Clamped>::FromDegrees(90.f);
    auto cam = omath::source::Camera({0, 0, 0}, omath::source::ViewAngles{}, {1920.f, 1080.f}, x, 0.01f, 1000.f);

    const auto projected = cam.WorldToScreen(cam.GetViewProjectionMatrix(), {1000, 0, 50});
    std::print("{} {} {}", projected->x, projected->y, projected->z);
}