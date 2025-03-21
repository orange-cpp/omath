//
// Created by Vlad on 3/19/2025.
//


#include <iostream>
#include <print>
#include <omath/engines/opengl_engine/formulas.hpp>


int main()
{
    std::println("OMATH Projection Matrix Builder");

    float fov = 0;
    float near = 0;
    float far = 0;
    float viewPortWidth = 0;
    float viewPortHeight = 0;

    std::print("Enter camera fov: ");
    std::cin >> fov;

    std::print("Enter camera z near: ");
    std::cin >> near;

    std::print("Enter camera z far: ");
    std::cin >> far;

    std::print("Enter camera screen width: ");
    std::cin >> viewPortWidth;

    std::print("Enter camera screen height: ");
    std::cin >> viewPortHeight;

    const auto mat =
            omath::opengl_engine::CalcPerspectiveProjectionMatrix(fov, viewPortWidth / viewPortHeight, near, far);

    std::print("{}", mat.ToString());
};
