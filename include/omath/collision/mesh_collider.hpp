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
        using NumericType = MeshType::NumericType;

        using VertexType = Vector3<NumericType>;
        explicit MeshCollider(MeshType mesh): m_mesh(std::move(mesh))
        {

        }

        [[nodiscard]]
        const Vector3<float>& find_furthest_vertex(const Vector3<float>& direction) const
        {
            return *std::ranges::max_element(m_mesh.m_vertex_buffer, [&direction](const auto& first, const auto& second)
            {
                return first.dot(direction) < second.dot(direction);
            });
        }

        [[nodiscard]]
        Vector3<float> find_abs_furthest_vertex(const Vector3<float>& direction) const
        {
            return m_mesh.vertex_to_world_space(find_furthest_vertex(direction));
        }

    private:
        MeshType m_mesh;
    };
} // namespace omath::collision