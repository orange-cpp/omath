//
// Created by Vlad on 11/9/2025.
//

#pragma once
#include "omath/engines/source_engine/traits/pred_engine_trait.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <vector>

namespace omath::collision
{
    class MeshCollider
    {
    public:
        MeshCollider(const std::vector<Vector3<float>>& vertexes, const Vector3<float> origin)
            : m_vertexes(vertexes), m_origin(origin)
        {
            if (m_vertexes.empty())
                throw std::runtime_error("Collider cannot have 0 vertexes");
        }
        std::vector<Vector3<float>> m_vertexes;
        Vector3<float> m_origin;
        source_engine::ViewAngles m_rotation;

        [[nodiscard]]
        source_engine::Mat4X4 to_world() const
        {
            return mat_translation(m_origin) * source_engine::rotation_matrix(m_rotation);
        }

        [[nodiscard]]
        const Vector3<float>& find_furthest_vertex(const Vector3<float>& direction) const
        {
            return *std::ranges::max_element(m_vertexes, [&direction](const auto& first, const auto& second)
                                             { return first.dot(direction) < second.dot(direction); });
        }
        [[nodiscard]]
        Vector3<float> find_abs_furthest_vertex(const Vector3<float>& direction) const
        {
            return vertex_to_world_space(find_furthest_vertex(direction));
        }
        [[nodiscard]] Vector3<float> vertex_to_world_space( const Vector3<float>& local_vertex) const
        {
            auto abs_vec = to_world() * mat_column_from_vector(local_vertex);

            return {abs_vec.at(0, 0), abs_vec.at(1, 0), abs_vec.at(2, 0)};
        }
    };
} // namespace omath::collision