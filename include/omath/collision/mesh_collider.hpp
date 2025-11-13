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
        using NumericType = typename MeshType::NumericType;

        using VertexType = Vector3<NumericType>;
        explicit MeshCollider(MeshType mesh): m_mesh(std::move(mesh))
        {
        }

        [[nodiscard]]
        const VertexType& find_furthest_vertex(const VertexType& direction) const
        {
            return *std::ranges::max_element(m_mesh.m_vertex_buffer, [&direction](const auto& first, const auto& second)
                                             { return first.dot(direction) < second.dot(direction); });
        }

        [[nodiscard]]
        VertexType find_abs_furthest_vertex(const VertexType& direction) const
        {
            return m_mesh.vertex_to_world_space(find_furthest_vertex(direction));
        }

        [[nodiscard]]
        const VertexType& get_origin() const
        {
            return m_mesh.get_origin();
        }
    private:
        MeshType m_mesh;
    };
} // namespace omath::collision