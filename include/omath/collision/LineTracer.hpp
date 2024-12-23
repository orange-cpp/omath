//
// Created by Orange on 11/13/2024.
//
#pragma once

#include "omath/Vector3.hpp"
#include "omath/Triangle3d.hpp"

namespace omath::collision
{
    class Ray
    {
    public:
        Vector3 start;
        Vector3 end;

        [[nodiscard]]
        Vector3 DirectionVector() const;

        [[nodiscard]]
        Vector3 DirectionVectorNormalized() const;
    };
    class LineTracer
    {
    public:
        LineTracer() = delete;


        [[nodiscard]]
        static bool CanTraceLine(const Ray& ray, const Triangle3d& triangle);


        // Realization of Möller–Trumbore intersection algorithm
        // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
        [[nodiscard]]
        static Vector3 GetRayHitPoint(const Ray& ray, const Triangle3d& triangle);
    };
}
