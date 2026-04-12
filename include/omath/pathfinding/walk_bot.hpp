//
// Created by orange on 4/12/2026.
//
#pragma once
#include "navigation_mesh.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <functional>
namespace omath::pathfinding
{
    class WalkBot
    {
    public:
        void update(const Vector3<float>& bot_position, const Vector3<float>& target_position, float min_node_distance) const;

    private:
        std::weak_ptr<NavigationMesh> m_mav_mesh;
        float m_min_distance_to_path_point;
        std::optional<std::function<void(const Vector3<float>&)>> m_on_next_path_node = nullptr;
    };
} // namespace omath::pathfinding