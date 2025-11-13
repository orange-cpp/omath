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

    template<class ColliderType>
    class GjkAlgorithm final
    {
        using VertexType = typename ColliderType::VertexType;

    public:
        [[nodiscard]]
        static VertexType find_support_vertex(const ColliderType& collider_a, const ColliderType& collider_b,
                                              const VertexType& direction)
        {
            return collider_a.find_abs_furthest_vertex(direction) - collider_b.find_abs_furthest_vertex(-direction);
        }

        [[nodiscard]]
        static bool is_collide(const ColliderType& collider_a, const ColliderType& collider_b)
        {
            return is_collide_with_simplex_info(collider_a, collider_b).hit;
        }

        [[nodiscard]]
        static GjkHitInfo<VertexType> is_collide_with_simplex_info(const ColliderType& collider_a,
                                                                   const ColliderType& collider_b)
        {
            auto support = find_support_vertex(collider_a, collider_b, {1, 0, 0});

            Simplex<VertexType> simplex;
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