//
// Created by Orange on 11/13/2024.
//
#pragma once

#include "omath/triangle.hpp"
#include "omath/vector3.hpp"

namespace omath::collision
{
    class Ray
    {
    public:
        Vector3<float> start;
        Vector3<float> end;
        bool infinite_length = false;
        [[nodiscard]]
        Vector3<float> DirectionVector() const;

        [[nodiscard]]
        Vector3<float> DirectionVectorNormalized() const;
    };
    class LineTracer
    {
    public:
        LineTracer() = delete;


        [[nodiscard]]
        static bool CanTraceLine(const Ray& ray, const Triangle<Vector3<float>>& triangle);


        // Realization of Möller–Trumbore intersection algorithm
        // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
        [[nodiscard]]
        static Vector3<float> GetRayHitPoint(const Ray& ray, const Triangle<Vector3<float>>& triangle);
    };
}
