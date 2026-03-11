//
// Created by Vlad on 28.07.2024.
//
#include "omath/pathfinding/navigation_mesh.hpp"
#include <algorithm>
#include <sstream>
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

    void NavigationMesh::set_event(const Vector3<float>& vertex, std::string event_id)
    {
        if (!m_vertex_map.contains(vertex))
            throw std::invalid_argument(std::format("Vertex '{}' not found", vertex));

        m_vertex_events[vertex] = std::move(event_id);
    }

    void NavigationMesh::clear_event(const Vector3<float>& vertex)
    {
        m_vertex_events.erase(vertex);
    }

    std::optional<std::string> NavigationMesh::get_event(const Vector3<float>& vertex) const noexcept
    {
        const auto it = m_vertex_events.find(vertex);
        if (it == m_vertex_events.end())
            return std::nullopt;
        return it->second;
    }

    // Serialization format per vertex line:
    //   x y z neighbor_count event_id
    // where event_id is "-" when no event is set.
    // Neighbor lines follow: nx ny nz

    std::string NavigationMesh::serialize() const noexcept
    {
        std::ostringstream oss;
        for (const auto& [vertex, neighbors] : m_vertex_map)
        {
            const auto event_it = m_vertex_events.find(vertex);
            const std::string& event = (event_it != m_vertex_events.end()) ? event_it->second : "-";

            oss << vertex.x << ' ' << vertex.y << ' ' << vertex.z << ' ' << neighbors.size() << ' ' << event << '\n';

            for (const auto& n : neighbors)
                oss << n.x << ' ' << n.y << ' ' << n.z << '\n';
        }
        return oss.str();
    }

    void NavigationMesh::deserialize(const std::string& raw)
    {
        m_vertex_map.clear();
        m_vertex_events.clear();
        std::istringstream iss(raw);

        Vector3<float> vertex;
        std::size_t neighbors_count;
        std::string event;
        while (iss >> vertex.x >> vertex.y >> vertex.z >> neighbors_count >> event)
        {
            std::vector<Vector3<float>> neighbors;
            neighbors.reserve(neighbors_count);
            for (std::size_t i = 0; i < neighbors_count; ++i)
            {
                Vector3<float> n;
                if (!(iss >> n.x >> n.y >> n.z))
                    throw std::runtime_error("Deserialize: Unexpected end of data.");
                neighbors.push_back(n);
            }
            m_vertex_map.emplace(vertex, std::move(neighbors));

            if (event != "-")
                m_vertex_events.emplace(vertex, std::move(event));
        }
    }
} // namespace omath::pathfinding
