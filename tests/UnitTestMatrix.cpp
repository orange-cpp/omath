//
// Created by vlad on 5/18/2024.
//
#include <gtest/gtest.h>
#include <uml/matrix.h>
#include <print>


TEST(UnitTestMatrix, ToString)
{
    uml::matrix matrix(2, 2);
    matrix.set(1.1);
    const auto str = matrix.to_string();

    std::cout << str;

}