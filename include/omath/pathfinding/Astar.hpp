//
// Created by Vlad on 28.07.2024.
//

#pragma once
#include <vector>
#include "NavigationMesh.hpp"
#include "omath/Vector3.hpp"
#include "omath/omath_export.hpp"

namespace omath::pathfinding
{
    class OMATH_API Astar final
    {
    public:
        [[nodiscard]]
        static std::vector<Vector3> FindPath(const Vector3& start, const Vector3& end, const NavigationMesh& navMesh);

    };
}