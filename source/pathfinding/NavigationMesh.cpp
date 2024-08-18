//
// Created by Vlad on 28.07.2024.
//
#include "omath/pathfinding/NavigationMesh.h"

#include <stdexcept>

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

    std::vector<uint8_t> NavigationMesh::Serialize() const
    {
        auto dumpToVector =[]<typename T>(const T& t, std::vector<uint8_t>& vec){
            for (size_t i = 0; i < sizeof(t); i++)
                vec.push_back(*(reinterpret_cast<uint8_t*>(&t)+i));
        };

        std::vector<uint8_t> raw;


        for (const auto& [vertex, neighbors] : m_verTextMap)
        {
            const uint16_t neighborsCount = neighbors.size();

            dumpToVector(vertex, raw);
            dumpToVector(neighborsCount, raw);

            for (const auto& neighbor : neighbors)
                dumpToVector(neighbor, raw);
        }
        return raw;

    }

    void NavigationMesh::Deserialize(const std::vector<uint8_t> &raw)
    {
        auto loadFromVector = [](const std::vector<uint8_t>& vec, size_t& offset, auto& value) {
            if (offset + sizeof(value) > vec.size()) {
                throw std::runtime_error("Deserialize: Invalid input data size.");
            }
            std::memcpy(&value, vec.data() + offset, sizeof(value));
            offset += sizeof(value);
        };

        m_verTextMap.clear();

        size_t offset = 0;

        while (offset < raw.size()) {
            Vector3 vertex;
            loadFromVector(raw, offset, vertex);

            uint16_t neighborsCount;
            loadFromVector(raw, offset, neighborsCount);

            std::vector<Vector3> neighbors;
            neighbors.reserve(neighborsCount);

            for (size_t i = 0; i < neighborsCount; ++i) {
                Vector3 neighbor;
                loadFromVector(raw, offset, neighbor);
                neighbors.push_back(neighbor);
            }

            m_verTextMap.emplace(vertex, std::move(neighbors));
        }
    }
}
