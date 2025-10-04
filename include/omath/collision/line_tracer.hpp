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
    };
} // namespace omath::collision
