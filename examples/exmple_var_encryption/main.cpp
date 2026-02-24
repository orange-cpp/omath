//
// Created by orange on 24.02.2026.
//
#include "omath/containers/encrypted_variable.hpp"
#include <omath/omath.hpp>
#include <print>
int main()
{
    OMATH_DEF_CRYPT_VAR(int, 64) var{5};
    var.encrypt();
    std::println("{}", var.value());
    var.decrypt();
    std::println("{}", var.value());
    return var.value();
}