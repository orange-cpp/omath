//
// Created by orange on 4/12/2026.
//
#include "omath/pathfinding/walk_bot.hpp"
#include "omath/pathfinding/a_star.hpp"

namespace omath::pathfinding
{

    void WalkBot::update(const Vector3<float>& bot_position, const Vector3<float>& target_position,
                         const float min_node_distance) const
    {
        const auto nav_mesh = m_mav_mesh.lock();

        if (!nav_mesh)
            return;

        const auto path = Astar::find_path(bot_position, target_position, *nav_mesh);

        if (path.empty())
            return;

        if (!m_on_next_path_node.has_value())
            return;

        if (path.size() > 1 && path.front().distance_to(bot_position) <= min_node_distance)
            m_on_next_path_node->operator()(path[1]);
        else
            m_on_next_path_node->operator()(path.front());
    }
    void WalkBot::on_path(std::function<void(const Vector3<float>&)> callback)
    {
        m_on_next_path_node = callback;
    }
} // namespace omath::pathfinding