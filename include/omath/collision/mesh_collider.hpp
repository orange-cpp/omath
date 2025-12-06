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

    private:
        [[nodiscard]]
        const VertexType& find_furthest_vertex(const VectorType& direction) const
        {
            return *std::ranges::max_element(
                    m_mesh.m_vertex_buffer, [&direction](const auto& first, const auto& second)
                    { return first.position.dot(direction) < second.position.dot(direction); });
        }
        MeshType m_mesh;
    };
} // namespace omath::collision