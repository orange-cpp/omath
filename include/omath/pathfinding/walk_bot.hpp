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
    class WalkBot
    {
    public:
        WalkBot() = default;
        explicit WalkBot(std::shared_ptr<NavigationMesh> mesh);

        void set_nav_mesh(std::shared_ptr<NavigationMesh> mesh);

        void update(const Vector3<float>& bot_position, const Vector3<float>& target_position, float min_node_distance);

        void on_path(std::function<void(const Vector3<float>&)> callback);

    private:
        std::weak_ptr<NavigationMesh> m_mav_mesh;
        std::optional<std::function<void(const Vector3<float>&)>> m_on_next_path_node;
        std::optional<Vector3<float>> m_last_visited;
    };
} // namespace omath::pathfinding