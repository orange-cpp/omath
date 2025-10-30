//
// Created by Vlad on 28.07.2024.
//
#include "omath/pathfinding/a_star.hpp"
#include <algorithm>
#include <optional>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace omath::pathfinding
{
    struct PathNode final
    {
        std::optional<Vector3<float>> came_from;
        float g_cost = 0.f;
    };

    struct OpenListNode final
    {
        Vector3<float> position;
        float f_cost;

        bool operator>(const OpenListNode& other) const noexcept
        {
            return f_cost > other.f_cost;
        }
    };

    std::vector<Vector3<float>>
    Astar::reconstruct_final_path(const std::unordered_map<Vector3<float>, PathNode>& closed_list,
                                  const Vector3<float>& current) noexcept
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

    std::vector<Vector3<float>> Astar::find_path(const Vector3<float>& start, const Vector3<float>& end,
                                                 const NavigationMesh& nav_mesh) noexcept
    {
        std::unordered_map<Vector3<float>, PathNode> closed_list;
        std::unordered_map<Vector3<float>, PathNode> node_data;
        std::priority_queue<OpenListNode, std::vector<OpenListNode>, std::greater<>> open_list;

        auto maybe_start_vertex = nav_mesh.get_closest_vertex(start);
        auto maybe_end_vertex = nav_mesh.get_closest_vertex(end);

        if (!maybe_start_vertex || !maybe_end_vertex)
            return {};

        const auto start_vertex = maybe_start_vertex.value();
        const auto end_vertex = maybe_end_vertex.value();

        node_data.emplace(start_vertex, PathNode{std::nullopt, 0.f});
        open_list.push({start_vertex, start_vertex.distance_to(end_vertex)});

        while (!open_list.empty())
        {
            auto current = open_list.top().position;
            open_list.pop();

            if (closed_list.contains(current))
                continue;

            auto current_node_it = node_data.find(current);
            if (current_node_it == node_data.end())
                continue;

            const auto current_node = current_node_it->second;

            if (current == end_vertex)
                return reconstruct_final_path(closed_list, current);

            closed_list.emplace(current, current_node);

            for (const auto& neighbor: nav_mesh.get_neighbors(current))
            {
                if (closed_list.contains(neighbor))
                    continue;

                const float tentative_g_cost = current_node.g_cost + neighbor.distance_to(current);

                auto node_it = node_data.find(neighbor);

                if (node_it == node_data.end() || tentative_g_cost < node_it->second.g_cost)
                {
                    node_data[neighbor] = PathNode{current, tentative_g_cost};
                    const float f_cost = tentative_g_cost + neighbor.distance_to(end_vertex);
                    open_list.push({neighbor, f_cost});
                }
            }
        }

        return {};
    }
} // namespace omath::pathfinding
