//
// Created by Vlad on 28.07.2024.
//

#pragma once
#include <vector>
#include "navigation_mesh.hpp"
#include "omath/Vector3.hpp"

namespace omath::pathfinding
{
    struct PathNode;
    class Astar final
    {
    public:
        [[nodiscard]]
        static std::vector<Vector3<float>> FindPath(const Vector3<float>& start, const Vector3<float>& end,
                                                    const NavigationMesh& navMesh);

    private:
        [[nodiscard]]
        static std::vector<Vector3<float>>
        ReconstructFinalPath(const std::unordered_map<Vector3<float>, PathNode>& closedList,
                             const Vector3<float>& current);

        [[nodiscard]]
        static auto GetPerfectNode(const std::unordered_map<Vector3<float>, PathNode>& openList,
                                   const Vector3<float>& endVertex);
    };
} // namespace omath::pathfinding
