//
// Created by Vlad on 28.07.2024.
//

module;
#include <unordered_map>
#include <vector>

export module omath.pathfinding.a_star;

export import omath.linear_algebra.vector3;
export import omath.pathfining.navigation_mesh;

export namespace omath::pathfinding
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

        [[nodiscard]]
        static auto get_perfect_node(const std::unordered_map<Vector3<float>, PathNode>& open_list,
                                     const Vector3<float>& end_vertex) noexcept;
    };
} // namespace omath::pathfinding
