//
// Created by Orange on 11/13/2024.
//
#pragma once

#include "omath/linear_algebra/triangle.hpp"
#include "omath/linear_algebra/vector3.hpp"

namespace omath::collision
{
    class Ray
    {
    public:
        Vector3<float> start;
        Vector3<float> end;
        bool infinite_length = false;

        [[nodiscard]]
        Vector3<float> direction_vector() const noexcept;

        [[nodiscard]]
        Vector3<float> direction_vector_normalized() const noexcept;
    };
    class LineTracer
    {
    public:
        LineTracer() = delete;

        [[nodiscard]]
        static bool can_trace_line(const Ray& ray, const Triangle<Vector3<float>>& triangle) noexcept;

        // Realization of Möller–Trumbore intersection algorithm
        // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
        [[nodiscard]]
        static Vector3<float> get_ray_hit_point(const Ray& ray, const Triangle<Vector3<float>>& triangle) noexcept;

        template<class MeshType>
        [[nodiscard]]
        static Vector3<float> get_ray_hit_point(const Ray& ray, const MeshType& mesh) noexcept
        {
            Vector3<float> mesh_hit = ray.end;

            auto begin = mesh.m_element_buffer_object.cbegin();
            auto end = mesh.m_element_buffer_object.cend();
            for (auto current = begin; current < end; current = std::next(current))
            {
                auto face = mesh.make_face_in_world_space(current);

                auto ray_stop_point = get_ray_hit_point(ray, face);
                if (ray_stop_point.distance_to(ray.start) < mesh_hit.distance_to(ray.start))
                    mesh_hit = ray_stop_point;
            }

            return mesh_hit;
        }
    };
} // namespace omath::collision
