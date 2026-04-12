//
// Created by orange on 4/12/2026.
//
#include "omath/pathfinding/walk_bot.hpp"
#include "omath/pathfinding/a_star.hpp"

namespace omath::pathfinding
{

    WalkBot::WalkBot(std::shared_ptr<NavigationMesh> mesh) : m_mav_mesh(std::move(mesh)) {}

    void WalkBot::set_nav_mesh(std::shared_ptr<NavigationMesh> mesh)
    {
        m_mav_mesh = std::move(mesh);
    }

    void WalkBot::update(const Vector3<float>& bot_position, const Vector3<float>& target_position,
                         const float min_node_distance)
    {
        if (!m_on_next_path_node.has_value())
            return;

        const auto nav_mesh = m_mav_mesh.lock();
        if (!nav_mesh)
            return;

        const auto path = Astar::find_path(bot_position, target_position, *nav_mesh);
        if (path.empty())
            return;

        const auto& nearest = path.front();

        // Record the nearest node as visited once we are close enough to it.
        if (nearest.distance_to(bot_position) <= min_node_distance)
            m_last_visited = nearest;

        // If the nearest node was already visited, advance to the next one so
        // we never oscillate back to a node we just left.
        // If the bot was displaced (blown back), nearest will be an unvisited
        // node, so we route to it first before continuing forward.
        if (m_last_visited.has_value() && *m_last_visited == nearest && path.size() > 1)
            m_on_next_path_node->operator()(path[1]);
        else
            m_on_next_path_node->operator()(nearest);
    }
    void WalkBot::on_path(std::function<void(const Vector3<float>&)> callback)
    {
        m_on_next_path_node = callback;
    }
} // namespace omath::pathfinding