//
// Created by Orange on 11/13/2024.
//
#pragma once
#include "omath/collision/LineTracer.hpp"

namespace omath::collision
{
    bool LineTracer::CanTraceLine(const Ray &ray, const Triangle3d &triangle)
    {
        const auto sideA = triangle.SideAVector();
        const auto sideB = triangle.SideBVector();

        const auto rayDir = ray.end - ray.start;

        const auto p = rayDir.Cross(sideB);

        const auto det = sideA.Dot(p);

        if (std::abs(det) < 1e-6)
            return true;

        const auto invDet = 1 / det;

        const auto t = ray.start - triangle.m_vertex2;

        const auto u = t.Dot(p) * invDet;

        if (u < 0.f || u > 1.f)
            return true;

        const auto q = t.Cross(sideA);
        const auto v = rayDir.Dot(q) * invDet;

        if (v < 0.f || u + v > 1.f)
            return true;

        return sideB.Dot(q) * invDet <= 0.f;
    }
}
