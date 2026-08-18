//
// Created by Orange on 11/13/2024.
//
#pragma once

#include "omath/3d_primitives/aabb.hpp"
#include "omath/3d_primitives/obb.hpp"
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
        using AABBType = primitives::Aabb<typename RayType::VectorType::ContainedType>;
        using OBBType = primitives::Obb<typename RayType::VectorType::ContainedType>;

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

        // Slab method ray-AABB intersection
        // Returns the hit point on the AABB surface, or ray.end if no intersection
        [[nodiscard]]
        constexpr static auto get_ray_hit_point(const RayType& ray, const AABBType& aabb) noexcept
        {
            using T = typename RayType::VectorType::ContainedType;
            const auto dir = ray.direction_vector();

            auto t_min = -std::numeric_limits<T>::infinity();
            auto t_max = std::numeric_limits<T>::infinity();

            const auto process_axis = [&](const T& d, const T& origin, const T& box_min,
                                          const T& box_max) -> bool
            {
                constexpr T k_epsilon = std::numeric_limits<T>::epsilon();
                if (std::abs(d) < k_epsilon)
                    return origin >= box_min && origin <= box_max;

                const T inv = T(1) / d;
                T t0 = (box_min - origin) * inv;
                T t1 = (box_max - origin) * inv;
                if (t0 > t1)
                    std::swap(t0, t1);

                t_min = std::max(t_min, t0);
                t_max = std::min(t_max, t1);
                return t_min <= t_max;
            };

            if (!process_axis(dir.x, ray.start.x, aabb.min.x, aabb.max.x))
                return ray.end;
            if (!process_axis(dir.y, ray.start.y, aabb.min.y, aabb.max.y))
                return ray.end;
            if (!process_axis(dir.z, ray.start.z, aabb.min.z, aabb.max.z))
                return ray.end;

            // t_hit: use entry point if in front of origin, otherwise 0 (started inside)
            const T t_hit = std::max(T(0), t_min);

            if (t_max < T(0))
                return ray.end; // box entirely behind origin

            if (!ray.infinite_length && t_hit > T(1))
                return ray.end; // box beyond ray endpoint

            return ray.start + dir * t_hit;
        }

        // Slab method ray-OBB intersection. Project the ray into the OBB's local frame
        // (axes are orthonormal, so the inverse rotation is just a transpose / dot products),
        // then run the standard slab test against the local box [-half_extents, +half_extents].
        // The ray parameter t is invariant under rigid transform, so the hit point is recovered
        // in world space as ray.start + dir * t_hit.
        [[nodiscard]]
        constexpr static auto get_ray_hit_point(const RayType& ray, const OBBType& obb) noexcept
        {
            using T = typename RayType::VectorType::ContainedType;

            const auto offset = ray.start - obb.center;
            const auto dir = ray.direction_vector();

            const T local_start[3] = {offset.dot(obb.axis_x), offset.dot(obb.axis_y), offset.dot(obb.axis_z)};
            const T local_dir[3] = {dir.dot(obb.axis_x), dir.dot(obb.axis_y), dir.dot(obb.axis_z)};
            const T half[3] = {obb.half_extents.x, obb.half_extents.y, obb.half_extents.z};

            auto t_min = -std::numeric_limits<T>::infinity();
            auto t_max = std::numeric_limits<T>::infinity();

            const auto process_axis = [&](const T& d, const T& origin, const T& h) -> bool
            {
                constexpr T k_epsilon = std::numeric_limits<T>::epsilon();
                if (std::abs(d) < k_epsilon)
                    return origin >= -h && origin <= h;

                const T inv = T(1) / d;
                T t0 = (-h - origin) * inv;
                T t1 = (h - origin) * inv;
                if (t0 > t1)
                    std::swap(t0, t1);

                t_min = std::max(t_min, t0);
                t_max = std::min(t_max, t1);
                return t_min <= t_max;
            };

            if (!process_axis(local_dir[0], local_start[0], half[0]))
                return ray.end;
            if (!process_axis(local_dir[1], local_start[1], half[1]))
                return ray.end;
            if (!process_axis(local_dir[2], local_start[2], half[2]))
                return ray.end;

            const T t_hit = std::max(T(0), t_min);

            if (t_max < T(0))
                return ray.end; // box entirely behind origin

            if (!ray.infinite_length && t_hit > T(1))
                return ray.end; // box beyond ray endpoint

            return ray.start + dir * t_hit;
        }

        // Traces the mesh in its own local frame rather than lifting every face into world space. Doing it the old way
        // costs three 4x4 multiplies per face; folding the ray the other way costs two for the whole mesh, whatever
        // the face count. An affine transform preserves the ray parameter t, so the nearest hit found in local space
        // is the nearest hit in world space, and the world point is recovered from t on the original ray.
        // MeshCollider::find_furthest_vertex plays the same trick for support queries.
        template<class MeshType>
        [[nodiscard]]
        constexpr static auto get_ray_hit_point(const RayType& ray, const MeshType& mesh) noexcept
        {
            using VectorType = typename RayType::VectorType;

            const auto to_local = mesh.get_to_world_matrix().inverted();

            if (!to_local)
                return ray.end; // Degenerate world matrix - a zero scale collapses the mesh to nothing to hit

            const auto& inverse = *to_local;
            const auto to_local_space = [&inverse](const VectorType& point)
            {
                return VectorType{
                        inverse.at(0, 0) * point.x + inverse.at(0, 1) * point.y + inverse.at(0, 2) * point.z
                                + inverse.at(0, 3),
                        inverse.at(1, 0) * point.x + inverse.at(1, 1) * point.y + inverse.at(1, 2) * point.z
                                + inverse.at(1, 3),
                        inverse.at(2, 0) * point.x + inverse.at(2, 1) * point.y + inverse.at(2, 2) * point.z
                                + inverse.at(2, 3)};
            };

            const RayType local_ray{to_local_space(ray.start), to_local_space(ray.end), ray.infinite_length};

            // Distance along the local ray is |local_dir| * t, and |local_dir| is fixed for the whole loop, so
            // ordering candidates by local distance is ordering them by t - which is the same order world distance
            // would give. Squared distance keeps that comparison free of the sqrt the old world-space version paid
            // twice per face.
            auto best_local_hit = local_ray.end;
            auto best_distance_sqr = local_ray.start.distance_to_sqr(local_ray.end);

            const auto begin = mesh.m_element_buffer_object.cbegin();
            const auto end = mesh.m_element_buffer_object.cend();
            for (auto current = begin; current < end; current = std::next(current))
            {
                const auto face = mesh.make_face_in_local_space(current);

                const auto local_hit = get_ray_hit_point(local_ray, face);
                // ReSharper disable once CppTooWideScopeInitStatement
                const auto distance_sqr = local_hit.distance_to_sqr(local_ray.start);

                if (distance_sqr < best_distance_sqr)
                {
                    best_local_hit = local_hit;
                    best_distance_sqr = distance_sqr;
                }
            }

            if (best_local_hit == local_ray.end)
                return ray.end;

            // Recovered from the original world ray rather than pushed back through the world matrix, so the result
            // carries no round-trip error from the inverse.
            const auto local_dir = local_ray.direction_vector();
            const auto t_hit = (best_local_hit - local_ray.start).dot(local_dir) / local_dir.dot(local_dir);

            return ray.start + ray.direction_vector() * t_hit;
        }
    };
} // namespace omath::collision
