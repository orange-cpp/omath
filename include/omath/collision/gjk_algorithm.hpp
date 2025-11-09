//
// Created by Vlad on 11/9/2025.
//

#pragma once
#include "mesh_collider.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include "simplex.hpp"

namespace omath::collision
{
    class GjkAlgorithm final
    {
    public:
        [[nodiscard]]
        static Vector3<float> find_support_vertex(const MeshCollider& collider_a, const MeshCollider& collider_b,
                                                  const Vector3<float>& direction)
        {
            return collider_a.find_abs_furthest_vertex(direction) - collider_b.find_abs_furthest_vertex(-direction);
        }

        [[nodiscard]]
        static bool check_collision(const MeshCollider& collider_a, const MeshCollider& collider_b)
        {
            // Get initial support point in any direction
            auto support = find_support_vertex(collider_a, collider_b, {1, 0, 0});

            Simplex points;
            points.push_front(support);

            auto direction = -support;

            while (true)
            {
                support = find_support_vertex(collider_a, collider_b, direction);

                if (support.dot(direction) <= 0.f)
                    return false;

                points.push_front(support);

                if (handle_simplex(points, direction))
                    return true;
            }
        }
    };
}// namespace omath::collision