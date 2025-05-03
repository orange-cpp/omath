//
// Created by Vlad on 28.07.2024.
//

#pragma once
#include "omath/pathfinding/navigation_mesh.hpp"
#include "omath/vector3.hpp"
#include <vector>

namespace omath::pathfinding
{
    struct PathNode;
    class Astar final
    {
    public:
        [[nodiscard]]
        static std::vector<Vector3<float>> find_path(const Vector3<float>& start, const Vector3<float>& end,
                                                     const NavigationMesh& nav_mesh);

    private:
        [[nodiscard]]
        static std::vector<Vector3<float>>
        reconstruct_final_path(const std::unordered_map<Vector3<float>, PathNode>& closed_list,
                               const Vector3<float>& current);

        [[nodiscard]]
        static auto get_perfect_node(const std::unordered_map<Vector3<float>, PathNode>& open_list,
                                     const Vector3<float>& endVertex);
    };
} // namespace omath::pathfinding
