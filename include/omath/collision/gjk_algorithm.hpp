//
// Created by Vlad on 11/9/2025.
//

#pragma once
#include "mesh_collider.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include "simplex.hpp"

namespace omath::collision
{
    template<class ColliderType = MeshCollider<>>
    class GjkAlgorithm final
    {
    public:
        [[nodiscard]]
        static ColliderType::VertexType find_support_vertex(const ColliderType& collider_a,
                                                            const ColliderType& collider_b,
                                                            const ColliderType::VertexType& direction)
        {
            return collider_a.find_abs_furthest_vertex(direction) - collider_b.find_abs_furthest_vertex(-direction);
        }

        [[nodiscard]]
        static bool is_collide(const ColliderType& collider_a, const ColliderType& collider_b)
        {
            // Get initial support point in any direction
            auto support = find_support_vertex(collider_a, collider_b, {1, 0, 0});

            Simplex<typename ColliderType::VertexType> simplex;
            simplex.push_front(support);

            auto direction = -support;

            while (true)
            {
                support = find_support_vertex(collider_a, collider_b, direction);

                if (support.dot(direction) <= 0.f)
                    return false;

                simplex.push_front(support);

                if (simplex.handle(direction))
                    return true;
            }
        }
    };
} // namespace omath::collision