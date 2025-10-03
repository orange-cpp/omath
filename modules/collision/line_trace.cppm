//
// Created by Vlad on 10/3/2025.
//

export module omath.collision.line_trace;

export import omath.linear_algebra.vector3;
export import omath.collision.triangle;

export namespace omath::collision
{
    class Ray final
    {
    public:
        Vector3<float> start{};
        Vector3<float> end{};
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
