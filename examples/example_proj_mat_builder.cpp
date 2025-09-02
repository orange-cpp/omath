//
// Created by Vlad on 3/19/2025.
//


#include <iostream>
#include <print>
#include <sstream>
import omath.opengl_engine.formulas;


int main()
{
    std::println("OMATH Projection Matrix Builder");

    float fov = 0;
    float near = 0;
    float far = 0;
    float view_port_width = 0;
    float view_port_height = 0;

    std::print("Enter camera fov: ");
    std::cin >> fov;

    std::print("Enter camera z near: ");
    std::cin >> near;

    std::print("Enter camera z far: ");
    std::cin >> far;

    std::print("Enter camera screen width: ");
    std::cin >> view_port_width;

    std::print("Enter camera screen height: ");
    std::cin >> view_port_height;

    const auto mat =
            omath::opengl_engine::calc_perspective_projection_matrix(fov, view_port_width / view_port_height, near, far);

    std::print("{}", mat);
};
