//
// Created by Vladislav on 11.01.2026.
//
#include "omath/3d_primitives/box.hpp"
#include "omath/engines/opengl_engine/primitives.hpp"
#include <gtest/gtest.h>

TEST(test, test)
{
    auto result = omath::primitives::create_box<omath::opengl_engine::BoxMesh>({0.f, 30.f, 0.f}, {}, omath::opengl_engine::k_abs_forward,
                                                omath::opengl_engine::k_abs_right);
}