//
// Created by vlad on 5/18/2024.
//
#include <gtest/gtest.h>
#include <omath/Matrix.h>
#include <print>


TEST(UnitTestMatrix, ToString)
{
    omath::Matrix matrix(2, 2);
    matrix.Set(1.1);
    const auto str = matrix.ToSrtring();

    std::cout << str;

}