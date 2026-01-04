//
// Created by Vlad on 28.07.2024.
//
#include "omath/pathfinding/navigation_mesh.hpp"
#include <algorithm>
#include <cstring>
#include <limits>
#include <ranges>
#include <stdexcept>
namespace omath::pathfinding
{
    std::expected<Vector3<float>, std::string>
    NavigationMesh::get_closest_vertex(const Vector3<float>& point) const noexcept
    {
        const auto res = std::ranges::min_element(m_vertex_map, [&point](const auto& a, const auto& b)
                                                  { return a.first.distance_to(point) < b.first.distance_to(point); });

        if (res == m_vertex_map.cend())
            return std::unexpected("Failed to get closest point");

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
        std::vector<std::uint8_t> raw;

        // Pre-calculate total size for better performance
        std::size_t total_size = 0;
        for (const auto& [vertex, neighbors] : m_vertex_map)
        {
            total_size += sizeof(vertex) + sizeof(std::uint16_t) + sizeof(Vector3<float>) * neighbors.size();
        }
        raw.reserve(total_size);

        auto dump_to_vector = [&raw]<typename T>(const T& t)
        {
            const auto* byte_ptr = reinterpret_cast<const std::uint8_t*>(&t);
            raw.insert(raw.end(), byte_ptr, byte_ptr + sizeof(T));
        };

        for (const auto& [vertex, neighbors] : m_vertex_map)
        {
            // Clamp neighbors count to fit in uint16_t (prevents silent data corruption)
            // NOTE: If neighbors.size() > 65535, only the first 65535 neighbors will be serialized.
            // This is a limitation of the current serialization format using uint16_t for count.
            const auto clamped_count =
                    std::min<std::size_t>(neighbors.size(), std::numeric_limits<std::uint16_t>::max());
            const auto neighbors_count = static_cast<std::uint16_t>(clamped_count);

            dump_to_vector(vertex);
            dump_to_vector(neighbors_count);

            // Only serialize up to the clamped count
            for (const auto& neighbor : neighbors | std::views::take(clamped_count))
            {
                dump_to_vector(neighbor);
            }
        }
        return raw;
    }

    void NavigationMesh::deserialize(const std::vector<uint8_t>& raw) noexcept
    {
        auto load_from_vector = [](const std::vector<uint8_t>& vec, std::size_t& offset, auto& value)
        {
            if (offset + sizeof(value) > vec.size())
                throw std::runtime_error("Deserialize: Invalid input data size.");

            std::copy_n(vec.data() + offset, sizeof(value), reinterpret_cast<uint8_t*>(&value));
            offset += sizeof(value);
        };

        m_vertex_map.clear();

        std::size_t offset = 0;

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
