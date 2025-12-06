//
// Created by Vlad on 11/9/2025.
//

#pragma once
#include "simplex.hpp"

namespace omath::collision
{
    template<class VertexType>
    struct GjkHitInfo final
    {
        bool hit{false};
        Simplex<VertexType> simplex; // valid only if hit == true and size==4
    };

    template<class ColliderInterfaceType>
    class GjkAlgorithm final
    {
        using VectorType = ColliderInterfaceType::VectorType;

    public:
        [[nodiscard]]
        static VectorType find_support_vertex(const ColliderInterfaceType& collider_a,
                                              const ColliderInterfaceType& collider_b, const VectorType& direction)
        {
            return collider_a.find_abs_furthest_vertex_position(direction)
                   - collider_b.find_abs_furthest_vertex_position(-direction);
        }

        [[nodiscard]]
        static bool is_collide(const ColliderInterfaceType& collider_a, const ColliderInterfaceType& collider_b)
        {
            return is_collide_with_simplex_info(collider_a, collider_b).hit;
        }

        [[nodiscard]]
        static GjkHitInfo<VectorType> is_collide_with_simplex_info(const ColliderInterfaceType& collider_a,
                                                                   const ColliderInterfaceType& collider_b)
        {
            auto support = find_support_vertex(collider_a, collider_b, VectorType{1, 0, 0});

            Simplex<VectorType> simplex;
            simplex.push_front(support);

            auto direction = -support;

            while (true)
            {
                support = find_support_vertex(collider_a, collider_b, direction);

                if (support.dot(direction) <= 0.f)
                    return {false, simplex};

                simplex.push_front(support);

                if (simplex.handle(direction))
                    return {true, simplex};
            }
        }
    };
} // namespace omath::collision