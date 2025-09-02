//
// Created by Vlad on 9/3/2025.
//
module;
#include <vector>
#include <unordered_map>
#include <expected>
#include <string>
#include <stdexcept>
export module omath.pathfinding.navigation_mesh;
export import omath.vector3;

export namespace omath::pathfinding
{

    enum Error
    {
    };

    class NavigationMesh final
    {
    public:
        [[nodiscard]]
        std::expected<Vector3<float>, std::string> get_closest_vertex(const Vector3<float>& point) const noexcept
        {
            const auto res = std::ranges::min_element(m_vertex_map, [&point](const auto& a, const auto& b)
                                                      { return a.first.distance_to(point) < b.first.distance_to(point); });

            if (res == m_vertex_map.cend())
                return std::unexpected("Failed to get clossest point");

            return res->first;
        }

        [[nodiscard]]
        const std::vector<Vector3<float>>& get_neighbors(const Vector3<float>& vertex) const noexcept
        {
            return m_vertex_map.at(vertex);
        }

        [[nodiscard]]
        bool empty() const
        {
            return m_vertex_map.empty();
        }

        [[nodiscard]] std::vector<uint8_t> serialize() const noexcept
        {
            auto dump_to_vector = []<typename T>(const T& t, std::vector<uint8_t>& vec)
            {
                for (size_t i = 0; i < sizeof(t); i++)
                    vec.push_back(*(reinterpret_cast<const uint8_t*>(&t) + i));
            };

            std::vector<uint8_t> raw;

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

        void deserialize(const std::vector<uint8_t>& raw) noexcept
        {
            auto load_from_vector = [](const std::vector<uint8_t>& vec, size_t& offset, auto& value)
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

                uint16_t neighbors_count;
                load_from_vector(raw, offset, neighbors_count);

                std::vector<Vector3<float>> neighbors;
                neighbors.reserve(neighbors_count);

                for (size_t i = 0; i < neighbors_count; ++i)
                {
                    Vector3<float> neighbor;
                    load_from_vector(raw, offset, neighbor);
                    neighbors.push_back(neighbor);
                }

                m_vertex_map.emplace(vertex, std::move(neighbors));
            }
        }

        std::unordered_map<Vector3<float>, std::vector<Vector3<float>>> m_vertex_map;
    };
} // namespace omath::pathfinding
