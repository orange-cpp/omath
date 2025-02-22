//
// Created by Vlad on 28.07.2024.
//

#pragma once
#include <vector>
#include "NavigationMesh.hpp"
#include "omath/Vector3.hpp"

namespace omath::pathfinding
{
    struct PathNode;
    class Astar final
    {
    public:
        [[nodiscard]]
        static std::vector<Vector3> FindPath(const Vector3& start, const Vector3& end, const NavigationMesh& navMesh);
    private:
        [[nodiscard]]
        static std::vector<Vector3> ReconstructFinalPath(const std::unordered_map<Vector3, PathNode>& closedList, const Vector3& current);

        [[nodiscard]]
        static auto GetPerfectNode(const std::unordered_map<Vector3, PathNode>& openList, const Vector3& endVertex);
    };
}