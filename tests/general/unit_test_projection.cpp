//
// Created by Vlad on 27.08.2024.
//
#include <complex>
#include <gtest/gtest.h>
#include <omath/engines/source_engine/camera.hpp>
#include <omath/projection/camera.hpp>
#include <print>

TEST(UnitTestProjection, Projection)
{
    const auto x = omath::Angle<float, 0.f, 180.f, omath::AngleFlags::Clamped>::from_degrees(90.f);
    auto cam = omath::source_engine::Camera({0, 0, 0}, omath::source_engine::ViewAngles{}, {1920.f, 1080.f}, x, 0.01f, 1000.f);

    const auto projected = cam.world_to_screen({1000, 0, 50});
    std::print("{} {} {}", projected->x, projected->y, projected->z);
}