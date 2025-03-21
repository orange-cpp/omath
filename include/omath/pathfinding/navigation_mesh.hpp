//
// Created by Vlad on 28.07.2024.
//

#pragma once

#include <expected>
#include <string>
#include <vector>
#include "omath/vector3.hpp"

namespace omath::pathfinding
{

    enum Error
    {

    };

    class NavigationMesh final
    {
    public:
        [[nodiscard]]
        std::expected<Vector3<float>, std::string> GetClosestVertex(const Vector3<float>& point) const;

        [[nodiscard]]
        const std::vector<Vector3<float>>& GetNeighbors(const Vector3<float>& vertex) const;

        [[nodiscard]]
        bool Empty() const;

        [[nodiscard]] std::vector<uint8_t> Serialize() const;

        void Deserialize(const std::vector<uint8_t>& raw);

        std::unordered_map<Vector3<float>, std::vector<Vector3<float>>> m_verTextMap;
    };
} // namespace omath::pathfinding
