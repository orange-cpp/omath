//
// Created by Vlad on 18.08.2024.
//
#include <gtest/gtest.h>
#include <omath/pathfinding/Astar.hpp>


TEST(UnitTestAstar, FindingRightPath)
{
    omath::pathfinding::NavigationMesh mesh;

    mesh.m_verTextMap[{0.f, 0.f, 0.f}] = {{0.f, 1.f, 0.f}};
    mesh.m_verTextMap[{0.f, 1.f, 0.f}] = {{0.f, 2.f, 0.f}};
    mesh.m_verTextMap[{0.f, 2.f, 0.f}] = {{0.f, 3.f, 0.f}};
    mesh.m_verTextMap[{0.f, 3.f, 0.f}] = {};
    std::ignore = omath::pathfinding::Astar::FindPath({}, {0.f, 3.f, 0.f}, mesh);
}