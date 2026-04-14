//
// Created by orange on 4/12/2026.
//
#pragma once
#include "navigation_mesh.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <functional>
#include <memory>
namespace omath::pathfinding
{
    enum class WalkBotStatus
    {
        IDLE,
        PATHING,
        FINISHED
    };
    class WalkBot
    {
    public:
        WalkBot() = default;
        explicit WalkBot(const std::shared_ptr<NavigationMesh>& mesh, float min_node_distance = 1.f);

        void set_nav_mesh(const std::shared_ptr<NavigationMesh>& mesh);
        void set_min_node_distance(float distance);

        void set_target(const Vector3<float>& target);

        // Clear navigation state so the bot can be re-routed without stale
        // visited-node memory.
        void reset();

        // Call every game tick with the current bot world position.
        void update(const Vector3<float>& bot_position);

        void on_path(const std::function<void(const Vector3<float>&)>& callback);
        void on_status(const std::function<void(WalkBotStatus)>& callback);

    private:
        std::weak_ptr<NavigationMesh> m_nav_mesh;
        std::optional<std::function<void(const Vector3<float>&)>> m_on_next_path_node;
        std::optional<std::function<void(WalkBotStatus)>> m_on_status_update;
        std::optional<Vector3<float>> m_last_visited;
        std::optional<Vector3<float>> m_target;
        float m_min_node_distance{1.f};
    };
} // namespace omath::pathfinding