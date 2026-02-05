//
// Created by Orange on 11/13/2024.
//
#pragma once

#include "omath/linear_algebra/triangle.hpp"
#include "omath/linear_algebra/vector3.hpp"

namespace omath::collision
{
    template<class T = Vector3<float>>
    class Ray final
    {
    public:
        using VectorType = T;
        VectorType start;
        VectorType end;
        bool infinite_length = false;

        [[nodiscard]]
        constexpr VectorType direction_vector() const noexcept
        {
            return end - start;
        }

        [[nodiscard]]
        constexpr VectorType direction_vector_normalized() const noexcept
        {
            return direction_vector().normalized();
        }
    };

    template<class RayType = Ray<>>
    class LineTracer final
    {
        using TriangleType = Triangle<typename RayType::VectorType>;

    public:
        LineTracer() = delete;

        [[nodiscard]]
        constexpr static bool can_trace_line(const RayType& ray, const TriangleType& triangle) noexcept
        {
            return get_ray_hit_point(ray, triangle) == ray.end;
        }

        // Realization of Möller–Trumbore intersection algorithm
        // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
        [[nodiscard]]
        constexpr static auto get_ray_hit_point(const RayType& ray, const TriangleType& triangle) noexcept
        {
            constexpr float k_epsilon = std::numeric_limits<float>::epsilon();

            const auto side_a = triangle.side_a_vector();
            const auto side_b = triangle.side_b_vector();

            const auto ray_dir = ray.direction_vector();

            const auto p = ray_dir.cross(side_b);
            const auto det = side_a.dot(p);

            if (std::abs(det) < k_epsilon)
                return ray.end;

            const auto inv_det = 1 / det;
            const auto t = ray.start - triangle.m_vertex2;
            const auto u = t.dot(p) * inv_det;

            if ((u < 0 && std::abs(u) > k_epsilon) || (u > 1 && std::abs(u - 1) > k_epsilon))
                return ray.end;

            const auto q = t.cross(side_a);
            // ReSharper disable once CppTooWideScopeInitStatement
            const auto v = ray_dir.dot(q) * inv_det;

            if ((v < 0 && std::abs(v) > k_epsilon) || (u + v > 1 && std::abs(u + v - 1) > k_epsilon))
                return ray.end;

            const auto t_hit = side_b.dot(q) * inv_det;

            if (ray.infinite_length && t_hit <= k_epsilon)
                return ray.end;

            if (t_hit <= k_epsilon || t_hit > 1 - k_epsilon)
                return ray.end;

            return ray.start + ray_dir * t_hit;
        }

        template<class MeshType>
        [[nodiscard]]
        constexpr static auto get_ray_hit_point(const RayType& ray, const MeshType& mesh) noexcept
        {
            auto mesh_hit = ray.end;

            const auto begin = mesh.m_element_buffer_object.cbegin();
            const auto end = mesh.m_element_buffer_object.cend();
            for (auto current = begin; current < end; current = std::next(current))
            {
                const auto face = mesh.make_face_in_world_space(current);

                auto ray_stop_point = get_ray_hit_point(ray, face);
                if (ray_stop_point.distance_to(ray.start) < mesh_hit.distance_to(ray.start))
                    mesh_hit = ray_stop_point;
            }

            return mesh_hit;
        }
    };
} // namespace omath::collision
