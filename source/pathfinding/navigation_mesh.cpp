//
// Created by Vlad on 28.07.2024.
//
module;
#include <algorithm>
#include <stdexcept>
#include <expected>
#include <vector>
#include <cstdint>
module omath.pathfining.navigation_mesh;
namespace omath::pathfinding
{
    std::expected<Vector3<float>, std::string>
    NavigationMesh::get_closest_vertex(const Vector3<float>& point) const noexcept
    {
        const auto res = std::ranges::min_element(m_vertex_map, [&point](const auto& a, const auto& b)
                                                  { return a.first.distance_to(point) < b.first.distance_to(point); });

        if (res == m_vertex_map.cend())
            return std::unexpected("Failed to get clossest point");

        return res->first;
    }

    const std::vector<Vector3<float>>& NavigationMesh::get_neighbors(const Vector3<float>& vertex) const noexcept
    {
        return m_vertex_map.at(vertex);
    }

    bool NavigationMesh::empty() const
    {
        return m_vertex_map.empty();
    }

    std::vector<uint8_t> NavigationMesh::serialize() const noexcept
    {
        auto dump_to_vector = []<typename T>(const T& t, std::vector<uint8_t>& vec)
        {
            for (std::size_t i = 0; i < sizeof(t); i++)
                vec.push_back(*(reinterpret_cast<const uint8_t*>(&t) + i));
        };

        std::vector<std::uint8_t> raw;

        for (const auto& [vertex, neighbors]: m_vertex_map)
        {
            const auto neighbors_count = neighbors.size();

            dump_to_vector(vertex, raw);
            dump_to_vector(neighbors_count, raw);

            for (const auto& neighbor: neighbors)
                dump_to_vector(neighbor, raw);
        }
        return raw;
    }

    void NavigationMesh::deserialize(const std::vector<std::uint8_t>& raw) noexcept
    {
        auto load_from_vector = [](const std::vector<std::uint8_t>& vec, size_t& offset, auto& value)
        {
            if (offset + sizeof(value) > vec.size())
            {
                throw std::runtime_error("Deserialize: Invalid input data size.");
            }
            std::copy_n(vec.data() + offset, sizeof(value), reinterpret_cast<uint8_t*>(&value));
            offset += sizeof(value);
        };

        m_vertex_map.clear();

        size_t offset = 0;

        while (offset < raw.size())
        {
            Vector3<float> vertex;
            load_from_vector(raw, offset, vertex);

            std::uint16_t neighbors_count;
            load_from_vector(raw, offset, neighbors_count);

            std::vector<Vector3<float>> neighbors;
            neighbors.reserve(neighbors_count);

            for (std::size_t i = 0; i < neighbors_count; ++i)
            {
                Vector3<float> neighbor;
                load_from_vector(raw, offset, neighbor);
                neighbors.push_back(neighbor);
            }

            m_vertex_map.emplace(vertex, std::move(neighbors));
        }
    }
} // namespace omath::pathfinding
