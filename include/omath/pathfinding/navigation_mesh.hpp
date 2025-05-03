//
// Created by Vlad on 28.07.2024.
//

#pragma once

#include "omath/vector3.hpp"
#include <expected>
#include <string>
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
        std::expected<Vector3<float>, std::string> get_closest_vertex(const Vector3<float>& point) const;

        [[nodiscard]]
        const std::vector<Vector3<float>>& get_neighbors(const Vector3<float>& vertex) const;

        [[nodiscard]]
        bool empty() const;

        [[nodiscard]] std::vector<uint8_t> serialize() const;

        void deserialize(const std::vector<uint8_t>& raw);

        std::unordered_map<Vector3<float>, std::vector<Vector3<float>>> m_vertex_map;
    };
} // namespace omath::pathfinding
