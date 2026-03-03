//
// Created by Vlad on 11/9/2025.
//

#pragma once
#include "collider_interface.hpp"
#include "omath/linear_algebra/vector3.hpp"

#ifdef OMATH_BUILD_TESTS
// ReSharper disable once CppInconsistentNaming
class UnitTestColider_FindFurthestVertex_Test;
#endif

namespace omath::collision
{
    template<class MeshType>
    class MeshCollider final : public ColliderInterface<typename MeshType::VertexType::VectorType>
    {
#ifdef OMATH_BUILD_TESTS
        friend UnitTestColider_FindFurthestVertex_Test;
#endif
    public:
        using VertexType = MeshType::VertexType;
        using VectorType = MeshType::VertexType::VectorType;
        explicit MeshCollider(MeshType mesh): m_mesh(std::move(mesh))
        {
        }

        [[nodiscard]]
        VectorType find_abs_furthest_vertex_position(const VectorType& direction) const override
        {
            return m_mesh.vertex_position_to_world_space(find_furthest_vertex(direction).position);
        }

        [[nodiscard]]
        const VectorType& get_origin() const override
        {
            return m_mesh.get_origin();
        }
        void set_origin(const VectorType& new_origin) override
        {
            m_mesh.set_origin(new_origin);
        }

        [[nodiscard]]
        const MeshType& get_mesh() const
        {
            return m_mesh;
        }
        [[nodiscard]]
        MeshType& get_mesh()
        {
            return m_mesh;
        }
    private:
        [[nodiscard]]
        const VertexType& find_furthest_vertex(const VectorType& direction) const
        {
            // The support query arrives in world space, but vertex positions are stored
            // in local space.  We need argmax_v { world(v) · d }.
            //
            // world(v) = M·v  (ignoring translation, which is constant across vertices)
            // world(v) · d = v · Mᵀ·d
            //
            // So we transform the direction to local space once — O(1) — then compare
            // raw local positions, which is far cheaper than calling
            // vertex_position_to_world_space (full 4×4 multiply) for every vertex.
            //
            // d_local = upper-left 3×3 of M, transposed, times d_world:
            //   d_local[j] = sum_i  M.at(i,j) * d[i]   (i.e. column j of M dotted with d)
            const auto& m = m_mesh.get_to_world_matrix();
            const VectorType d_local = {
                    m[0, 0] * direction.x + m[1, 0] * direction.y + m[2, 0] * direction.z,
                    m[0, 1] * direction.x + m[1, 1] * direction.y + m[2, 1] * direction.z,
                    m[0, 2] * direction.x + m[1, 2] * direction.y + m[2, 2] * direction.z,
            };
            return *std::ranges::max_element(m_mesh.m_vertex_buffer, [&d_local](const auto& first, const auto& second)
                                             { return first.position.dot(d_local) < second.position.dot(d_local); });
        }
        MeshType m_mesh;
    };
} // namespace omath::collision