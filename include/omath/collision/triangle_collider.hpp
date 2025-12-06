//
// Created by Vlad on 11/9/2025.
//

#pragma once
#include "omath/linear_algebra/vector3.hpp"
#include <omath/linear_algebra/triangle.hpp>

namespace omath::collision
{
    struct TriangleCollider
    {
        Vector3<float> position;
    };
    class TriangleMeshCollider final : public ColliderInterface<Vector3<float>>
    {
    public:
        using VectorType = Vector3<float>;
        using VertexType = TriangleCollider;
        explicit TriangleMeshCollider(const Triangle<Vector3<float>>& triangle)
            : m_mesh({triangle.m_vertex1, triangle.m_vertex2, triangle.m_vertex3})
        {
        }

        [[nodiscard]]
        VectorType find_abs_furthest_vertex_position(const VectorType& direction) const override
        {
            return find_furthest_vertex(direction).position;
        }

        [[nodiscard]]
        const VectorType& get_origin() const override
        {
            return m_origin;
        }
        void set_origin(const VectorType&) override
        {

        }
    private:
        [[nodiscard]]
        const VertexType& find_furthest_vertex(const VectorType& direction) const
        {
            return *std::ranges::max_element(
                    m_mesh, [&direction](const auto& first, const auto& second)
                    { return first.position.dot(direction) < second.position.dot(direction); });
        }

        Vector3<float> m_origin{};
        std::array<VertexType, 3> m_mesh;
    };
} // namespace omath::collision