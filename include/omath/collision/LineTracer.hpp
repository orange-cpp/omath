//
// Created by Orange on 11/13/2024.
//
#pragma once
#include "omath/Vector3.hpp"
#include "omath/Triangle3d.hpp"
namespace omath::collision
{
    struct Ray
    {
        Vector3 start;
        Vector3 end;
    };
    class LineTracer
    {
    public:
        LineTracer() = delete;

        [[nodiscard]]
        static bool CanTraceLine(const Ray& ray, const Triangle3d& triangle);
    };
}
