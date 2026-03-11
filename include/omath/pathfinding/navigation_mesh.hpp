//
// Created by Vlad on 28.07.2024.
//

#pragma once

#include "omath/linear_algebra/vector3.hpp"
#include <expected>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace omath::pathfinding
{

    enum Error
    {
    };

    class NavigationMesh final
    {
    public:
        [[nodiscard]]
        std::expected<Vector3<float>, std::string> get_closest_vertex(const Vector3<float>& point) const noexcept;

        [[nodiscard]]
        const std::vector<Vector3<float>>& get_neighbors(const Vector3<float>& vertex) const noexcept;

        [[nodiscard]]
        bool empty() const;

        // Events -- per-vertex optional tag (e.g. "jump", "teleport")
        void set_event(const Vector3<float>& vertex, std::string event_id);
        void clear_event(const Vector3<float>& vertex);

        [[nodiscard]]
        std::optional<std::string> get_event(const Vector3<float>& vertex) const noexcept;

        [[nodiscard]] std::string serialize() const noexcept;

        void deserialize(const std::string& raw);

        std::unordered_map<Vector3<float>, std::vector<Vector3<float>>> m_vertex_map;

    private:
        std::unordered_map<Vector3<float>, std::string> m_vertex_events;
    };
} // namespace omath::pathfinding
