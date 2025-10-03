//
// Created by Orange on 11/13/2024.
//
module;
#include <cmath>
#include <limits>
module omath.collision.line_trace;

namespace omath::collision
{
    bool LineTracer::can_trace_line(const Ray& ray, const Triangle<Vector3<float>>& triangle) noexcept
    {
        return get_ray_hit_point(ray, triangle) == ray.end;
    }
    Vector3<float> Ray::direction_vector() const noexcept
    {
        return end - start;
    }

    Vector3<float> Ray::direction_vector_normalized() const noexcept
    {
        return direction_vector().normalized();
    }

    Vector3<float> LineTracer::get_ray_hit_point(const Ray& ray, const Triangle<Vector3<float>>& triangle) noexcept
    {
        constexpr float k_epsilon = std::numeric_limits<float>::epsilon();

        const auto side_a = triangle.side_a_vector();
        const auto side_b = triangle.side_b_vector();

        const auto ray_dir = ray.direction_vector();

        const auto p = ray_dir.cross(side_b);
        const auto det = side_a.dot(p);

        if (std::abs(det) < k_epsilon)
            return ray.end;

        const auto inv_det = 1.0f / det;
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

        if (t_hit <= k_epsilon || t_hit > 1.0f - k_epsilon)
            return ray.end;

        return ray.start + ray_dir * t_hit;
    }
} // namespace omath::collision
