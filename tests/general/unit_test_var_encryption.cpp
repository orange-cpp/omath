//
// Created by Vladislav on 04.01.2026.
//
#include "omath/linear_algebra/vector3.hpp"
#include <gtest/gtest.h>
#include <omath/containers/encrypted_variable.hpp>
TEST(Enc, Test)
{
    constexpr omath::Vector3<float> original = {1.f, 2.f, 3.f};
    OMATH_DEF_CRYPT_VAR(omath::Vector3<float>, 128) var{original};
    {
        omath::VarAnchor _ = var.drop_anchor();

        EXPECT_EQ(original, var.value());
    }
    EXPECT_NE(original, var.value());
}