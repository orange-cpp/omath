//
// Created by Vladislav on 04.01.2026.
//
#include "omath/linear_algebra/vector3.hpp"
#include <gtest/gtest.h>
#include <omath/containers/encrypted_variable.hpp>
TEST(Enc, Test)
{
    OMATH_DEF_CRYPT_VAR(omath::Vector3<float>, 128) var{{1.f, 2.f, 3.f}};
    OMATH_DEF_CRYPT_VAR(omath::Vector3<float>, 128) var2{{1.f, 2.f, 3.f}};
    {
        omath::Anchor _ = {var};
        omath::Anchor __ = {var2};

        std::println("Decrypted var1: {}", var.value());
        std::println("Decrypted var2: {}", var2.value());
    }
    std::println("Encrypted var1: {}", var.value());
    std::println("Encrypted var2: {}", var2.value());
}