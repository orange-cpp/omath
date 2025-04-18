//
// Created by Orange on 11/13/2024.
//
#include "omath/collision/line_tracer.hpp"

namespace omath::collision
{
    bool LineTracer::CanTraceLine(const Ray& ray, const Triangle<Vector3<float>>& triangle)
    {
        return GetRayHitPoint(ray, triangle) == ray.end;
    }
    Vector3<float> Ray::DirectionVector() const
    {
        return end - start;
    }

    Vector3<float> Ray::DirectionVectorNormalized() const
    {
        return DirectionVector().Normalized();
    }

    Vector3<float> LineTracer::GetRayHitPoint(const Ray& ray, const Triangle<Vector3<float>>& triangle)
    {
        constexpr float kEpsilon = std::numeric_limits<float>::epsilon();

        const auto sideA = triangle.SideAVector();
        const auto sideB = triangle.SideBVector();


        const auto rayDir = ray.DirectionVector();

        const auto p = rayDir.Cross(sideB);
        const auto det = sideA.Dot(p);


        if (std::abs(det) < kEpsilon)
            return ray.end;

        const auto invDet = 1.0f / det;
        const auto t = ray.start - triangle.m_vertex2;
        const auto u = t.Dot(p) * invDet;


        if ((u < 0 && std::abs(u) > kEpsilon) || (u > 1 && std::abs(u - 1) > kEpsilon))
            return ray.end;

        const auto q = t.Cross(sideA);
        const auto v = rayDir.Dot(q) * invDet;


        if ((v < 0 && std::abs(v) > kEpsilon) || (u + v > 1 && std::abs(u + v - 1) > kEpsilon))
            return ray.end;

        const auto tHit = sideB.Dot(q) * invDet;


        if (ray.infinite_length)
        {
            if (tHit <= kEpsilon)
                return ray.end;
        }
        else if (tHit <= kEpsilon || tHit > 1.0f - kEpsilon)
            return ray.end;

        return ray.start + rayDir * tHit;
    }
} // namespace omath::collision
