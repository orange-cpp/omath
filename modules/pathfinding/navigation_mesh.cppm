//
// Created by Vlad on 28.07.2024.
//

module;
#include <expected>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
export module omath.pathfining.navigation_mesh;

export import omath.linear_algebra.vector3;

export namespace omath::pathfinding
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

        [[nodiscard]] std::vector<uint8_t> serialize() const noexcept;

        void deserialize(const std::vector<uint8_t>& raw) noexcept;

        std::unordered_map<Vector3<float>, std::vector<Vector3<float>>> m_vertex_map;
    };
} // namespace omath::pathfinding
