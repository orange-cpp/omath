//
// Created by Vlad on 28.07.2024.
//
#include "omath/pathfinding/a_star.hpp"
#include <algorithm>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace omath::pathfinding
{
    struct PathNode final
    {
        std::optional<Vector3<float>> came_from;
        float g_cost = 0.f;
    };

    std::vector<Vector3<float>>
    Astar::reconstruct_final_path(const std::unordered_map<Vector3<float>, PathNode>& closed_list,
                                  const Vector3<float>& current)
    {
        std::vector<Vector3<float>> path;
        std::optional current_opt = current;

        while (current_opt)
        {
            path.push_back(*current_opt);

            auto it = closed_list.find(*current_opt);

            if (it == closed_list.end())
                break;

            current_opt = it->second.came_from;
        }

        std::ranges::reverse(path);
        return path;
    }
    auto Astar::get_perfect_node(const std::unordered_map<Vector3<float>, PathNode>& open_list,
                                 const Vector3<float>& endVertex)
    {
        return std::ranges::min_element(open_list,
                                        [&endVertex](const auto& a, const auto& b)
                                        {
                                            const float fa = a.second.g_cost + a.first.distance_to(endVertex);
                                            const float fb = b.second.g_cost + b.first.distance_to(endVertex);
                                            return fa < fb;
                                        });
    }

    std::vector<Vector3<float>> Astar::find_path(const Vector3<float>& start, const Vector3<float>& end,
                                                 const NavigationMesh& nav_mesh)
    {
        std::unordered_map<Vector3<float>, PathNode> closed_list;
        std::unordered_map<Vector3<float>, PathNode> open_list;

        auto maybe_start_vertex = nav_mesh.get_closest_vertex(start);
        auto maybe_end_vertex = nav_mesh.get_closest_vertex(end);

        if (!maybe_start_vertex || !maybe_end_vertex)
            return {};

        const auto start_vertex = maybe_start_vertex.value();
        const auto end_vertex = maybe_end_vertex.value();

        open_list.emplace(start_vertex, PathNode{std::nullopt, 0.f});

        while (!open_list.empty())
        {
            auto current_it = get_perfect_node(open_list, end_vertex);

            const auto current = current_it->first;
            const auto current_node = current_it->second;

            if (current == end_vertex)
                return reconstruct_final_path(closed_list, current);

            closed_list.emplace(current, current_node);
            open_list.erase(current_it);

            for (const auto& neighbor: nav_mesh.get_neighbors(current))
            {
                if (closed_list.contains(neighbor))
                    continue;

                const float tentative_g_cost = current_node.g_cost + neighbor.distance_to(current);

                const auto open_it = open_list.find(neighbor);

                if (open_it == open_list.end() || tentative_g_cost < open_it->second.g_cost)
                    open_list[neighbor] = PathNode{current, tentative_g_cost};
            }
        }

        return {};
    }
} // namespace omath::pathfinding
