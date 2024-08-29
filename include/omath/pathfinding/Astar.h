//
// Created by Vlad on 28.07.2024.
//

#pragma once
#include <vector>
#include "NavigationMesh.h"
#include "omath/Vector3.h"


namespace omath::pathfinding
{
    class Astar
    {
    public:
        [[nodiscard]]
        static std::vector<Vector3> FindPath(const Vector3& start, const Vector3& end, const NavigationMesh& navMesh);

    };
}