//
// Created by Vlad on 18.08.2024.
//
#include <gtest/gtest.h>
import omath.pathfinding.a_star;

TEST(unit_test_a_star, finding_right_path)
{
    omath::pathfinding::NavigationMesh mesh;

    mesh.m_vertex_map[{0.f, 0.f, 0.f}] = {{0.f, 1.f, 0.f}};
    mesh.m_vertex_map[{0.f, 1.f, 0.f}] = {{0.f, 2.f, 0.f}};
    mesh.m_vertex_map[{0.f, 2.f, 0.f}] = {{0.f, 3.f, 0.f}};
    mesh.m_vertex_map[{0.f, 3.f, 0.f}] = {};
    std::ignore = omath::pathfinding::Astar::find_path({}, {0.f, 3.f, 0.f}, mesh);
}