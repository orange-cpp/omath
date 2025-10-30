//
// Created by Vlad on 28.07.2024.
//

#pragma once
#include "omath/linear_algebra/vector3.hpp"
#include "omath/pathfinding/navigation_mesh.hpp"
#include <vector>

namespace omath::pathfinding
{
    struct PathNode;
    class Astar final
    {
    public:
        [[nodiscard]]
        static std::vector<Vector3<float>> find_path(const Vector3<float>& start, const Vector3<float>& end,
                                                     const NavigationMesh& nav_mesh) noexcept;

    private:
        [[nodiscard]]
        static std::vector<Vector3<float>>
        reconstruct_final_path(const std::unordered_map<Vector3<float>, PathNode>& closed_list,
                               const Vector3<float>& current) noexcept;
    };
} // namespace omath::pathfinding
