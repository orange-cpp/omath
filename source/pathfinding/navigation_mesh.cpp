//
// Created by Vlad on 28.07.2024.
//
#include "omath/pathfinding/navigation_mesh.hpp"

#include <algorithm>
#include <stdexcept>
namespace omath::pathfinding
{
    std::expected<Vector3<float>, std::string> NavigationMesh::GetClosestVertex(const Vector3<float> &point) const
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

    const std::vector<Vector3<float>>& NavigationMesh::GetNeighbors(const Vector3<float> &vertex) const
    {
        return m_verTextMap.at(vertex);
    }

    bool NavigationMesh::Empty() const
    {
        return m_verTextMap.empty();
    }

    std::vector<uint8_t> NavigationMesh::Serialize() const
    {
        auto dumpToVector =[]<typename T>(const T& t, std::vector<uint8_t>& vec){
            for (size_t i = 0; i < sizeof(t); i++)
                vec.push_back(*(reinterpret_cast<const uint8_t*>(&t)+i));
        };

        std::vector<uint8_t> raw;


        for (const auto& [vertex, neighbors] : m_verTextMap)
        {
            const auto neighborsCount = neighbors.size();

            dumpToVector(vertex, raw);
            dumpToVector(neighborsCount, raw);

            for (const auto& neighbor : neighbors)
                dumpToVector(neighbor, raw);
        }
        return raw;

    }

    void NavigationMesh::Deserialize(const std::vector<uint8_t> &raw)
    {
        auto loadFromVector = [](const std::vector<uint8_t>& vec, size_t& offset, auto& value)
        {
            if (offset + sizeof(value) > vec.size())
            {
                throw std::runtime_error("Deserialize: Invalid input data size.");
            }
            std::copy_n(vec.data() + offset, sizeof(value), (uint8_t*)&value);
            offset += sizeof(value);
        };

        m_verTextMap.clear();

        size_t offset = 0;

        while (offset < raw.size())
        {
            Vector3<float> vertex;
            loadFromVector(raw, offset, vertex);

            uint16_t neighborsCount;
            loadFromVector(raw, offset, neighborsCount);

            std::vector<Vector3<float>> neighbors;
            neighbors.reserve(neighborsCount);

            for (size_t i = 0; i < neighborsCount; ++i)
            {
                Vector3<float> neighbor;
                loadFromVector(raw, offset, neighbor);
                neighbors.push_back(neighbor);
            }

            m_verTextMap.emplace(vertex, std::move(neighbors));
        }
    }
}
