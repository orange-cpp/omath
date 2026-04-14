//
// Created by orange on 4/12/2026.
//
#include "omath/pathfinding/walk_bot.hpp"
#include "omath/pathfinding/a_star.hpp"

namespace omath::pathfinding
{

    WalkBot::WalkBot(const std::shared_ptr<NavigationMesh>& mesh, const float min_node_distance)
        : m_nav_mesh(mesh), m_min_node_distance(min_node_distance) {}

    void WalkBot::set_nav_mesh(const std::shared_ptr<NavigationMesh>& mesh)
    {
        m_nav_mesh = mesh;
    }

    void WalkBot::set_min_node_distance(const float distance)
    {
        m_min_node_distance = distance;
    }

    void WalkBot::set_target(const Vector3<float>& target)
    {
        m_target = target;
    }

    void WalkBot::reset()
    {
        m_last_visited.reset();
    }

    void WalkBot::update(const Vector3<float>& bot_position)
    {
        if (!m_target.has_value())
            return;

        if (m_target->distance_to(bot_position) <= m_min_node_distance)
        {
            if (m_on_status_update.has_value())
                m_on_status_update->operator()(WalkBotStatus::FINISHED);
            return;
        }

        if (!m_on_next_path_node.has_value())
            return;

        const auto nav_mesh = m_nav_mesh.lock();
        if (!nav_mesh)
        {
            if (m_on_status_update.has_value())
                m_on_status_update->operator()(WalkBotStatus::IDLE);
            return;
        }

        const auto path = Astar::find_path(bot_position, *m_target, *nav_mesh);
        if (path.empty())
        {
            if (m_on_status_update.has_value())
                m_on_status_update->operator()(WalkBotStatus::IDLE);
            return;
        }

        const auto& nearest = path.front();

        // Record the nearest node as visited once we are close enough to it.
        if (nearest.distance_to(bot_position) <= m_min_node_distance)
            m_last_visited = nearest;

        // If the nearest node was already visited, advance to the next one so
        // we never oscillate back to a node we just left.
        // If the bot was displaced (blown back), nearest will be an unvisited
        // node, so we route to it first before continuing forward.
        if (m_last_visited.has_value() && *m_last_visited == nearest && path.size() > 1)
            m_on_next_path_node->operator()(path[1]);
        else
            m_on_next_path_node->operator()(nearest);

        if (m_on_status_update.has_value())
            m_on_status_update->operator()(WalkBotStatus::PATHING);
    }

    void WalkBot::on_path(const std::function<void(const Vector3<float>&)>& callback)
    {
        m_on_next_path_node = callback;
    }

    void WalkBot::on_status(const std::function<void(WalkBotStatus)>& callback)
    {
        m_on_status_update = callback;
    }
} // namespace omath::pathfinding