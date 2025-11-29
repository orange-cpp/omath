//
// Created by Vlad on 11/9/2025.
//

#pragma once
#include "omath/linear_algebra/vector3.hpp"

namespace omath::collision
{
    template<class MeshType>
    class MeshCollider
    {
    public:

        using VertexType = MeshType::VertexType;
        using VectorType = VertexType::VectorType;
        explicit MeshCollider(MeshType mesh): m_mesh(std::move(mesh))
        {
        }

        [[nodiscard]]
        const VertexType& find_furthest_vertex(const VectorType& direction) const
        {
            return *std::ranges::max_element(
                    m_mesh.m_vertex_buffer, [&direction](const auto& first, const auto& second)
                    { return first.position.dot(direction) < second.position.dot(direction); });
        }

        [[nodiscard]]
        VertexType find_abs_furthest_vertex(const VectorType& direction) const
        {
            const auto& vertex = find_furthest_vertex(direction);
            auto new_vertex = vertex;
            new_vertex.position = m_mesh.vertex_to_world_space(find_furthest_vertex(direction).position);
            return new_vertex;
        }

        [[nodiscard]]
        const VectorType& get_origin() const
        {
            return m_mesh.get_origin();
        }

    private:
        MeshType m_mesh;
    };
} // namespace omath::collision