//
// Created by Vlad on 28.07.2024.
//
#include "omath/pathfinding/NavigationMesh.h"

namespace omath::pathfinding
{
    std::expected<Vector3, std::string> NavigationMesh::GetClossestVertex(const Vector3 &point) const
    {
        const auto res = std::ranges::min_element(m_verTextMap,
            [&point](const auto& a, const auto& b)
            {
                return a.first.DistTo(point) < b.first.DistTo(point);
            });

        if (res == m_verTextMap.cend())
            return std::unexpected("Failed to get clossest point");

        return res->first;
    }

    const std::vector<Vector3>& NavigationMesh::GetNeighbors(const Vector3 &vertex) const
    {
        return m_verTextMap.at(vertex);
    }
}
