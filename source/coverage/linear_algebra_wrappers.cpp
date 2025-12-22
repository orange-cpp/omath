// Small TU that explicitly instantiates and calls header non-inlined helpers
#include <omath/linear_algebra/vector3.hpp>
#include <omath/linear_algebra/vector4.hpp>
#include <omath/linear_algebra/triangle.hpp>

extern "C" void call_linear_algebra_wrappers()
{
    using Vec3 = omath::Vector3<float>;
    using Vec4 = omath::Vector4<float>;
    using Tri3 = omath::Triangle<Vec3>;

    Vec3 a{3.0f, 4.0f, 12.0f};
    Vec3 b{1.0f, 2.0f, 2.0f};
    Vec3 c{0.0f, 0.0f, 1.0f};

    volatile float l = a.length();
    (void)l;

    volatile auto ang = a.angle_between(b);
    (void)ang;

    volatile bool perp = a.is_perpendicular(b);
    (void)perp;

    Tri3 tri{a, b, c};
    volatile auto n = tri.calculate_normal();
    (void)n;

    Vec4 v4{2.0f, 3.0f, 5.0f, 7.0f};
    volatile float sum = v4.sum();
    (void)sum;
}
#include "coverage_wrappers.hpp"

namespace coverage_wrappers
{
    float vector2_distance(const omath::Vector2<float>& a, const omath::Vector2<float>& b)
    {
        return a.distance_to(b);
    }

    float vector2_dot(const omath::Vector2<float>& a, const omath::Vector2<float>& b)
    {
        return a.dot(b);
    }

    omath::Vector3<float> vector3_cross(const omath::Vector3<float>& a, const omath::Vector3<float>& b)
    {
        return a.cross(b);
    }

    float vector3_angle_between_deg(const omath::Vector3<float>& a, const omath::Vector3<float>& b)
    {
        return a.angle_between(b).value().as_degrees();
    }

    float vector4_dot(const omath::Vector4<float>& a, const omath::Vector4<float>& b)
    {
        return a.dot(b);
    }

    float vector2_normalized_length(const omath::Vector2<float>& a)
    {
        return a.normalized().length();
    }

    float vector2_abs_sum(const omath::Vector2<float>& a)
    {
        auto tmp = a;
        tmp.abs();
        return tmp.sum();
    }

    std::tuple<float, float> vector2_as_tuple(const omath::Vector2<float>& a)
    {
        return a.as_tuple();
    }

    float vector3_length(const omath::Vector3<float>& a)
    {
        return a.length();
    }

    float vector3_length_2d(const omath::Vector3<float>& a)
    {
        return a.length_2d();
    }

    bool vector3_is_perpendicular(const omath::Vector3<float>& a, const omath::Vector3<float>& b)
    {
        return a.is_perpendicular(b);
    }

    bool vector3_point_to_same_direction(const omath::Vector3<float>& a, const omath::Vector3<float>& b)
    {
        return a.point_to_same_direction(b);
    }

    float vector4_clamp_x(const omath::Vector4<float>& a, float minv, float maxv)
    {
        auto tmp = a;
        tmp.clamp(minv, maxv);
        return tmp.x;
    }

    float vector4_abs_sum(const omath::Vector4<float>& a)
    {
        auto tmp = a;
        tmp.abs();
        return tmp.sum();
    }

    float triangle_hypot(const omath::Triangle<omath::Vector3<float>>& t)
    {
        return t.hypot();
    }

    omath::Vector3<float> triangle_midpoint(const omath::Triangle<omath::Vector3<float>>& t)
    {
        return t.mid_point();
    }

    bool triangle_is_rectangular(const omath::Triangle<omath::Vector3<float>>& t)
    {
        return t.is_rectangular();
    }

    float mat_det_2x2(const omath::Mat<2,2>& m)
    {
        return m.determinant();
    }

    omath::Vector2<float> vector2_add(const omath::Vector2<float>& a, const omath::Vector2<float>& b)
    {
        return a + b;
    }

    omath::Vector2<float> vector2_sub(const omath::Vector2<float>& a, const omath::Vector2<float>& b)
    {
        return a - b;
    }

    omath::Vector2<float> vector2_mul_scalar(const omath::Vector2<float>& a, float s)
    {
        return a * s;
    }

    omath::Vector2<float> vector2_div_scalar(const omath::Vector2<float>& a, float s)
    {
        return a / s;
    }

    float vector2_length_sqr(const omath::Vector2<float>& a)
    {
        return a.length_sqr();
    }

    omath::Vector2<float> vector2_negate(const omath::Vector2<float>& a)
    {
        return -a;
    }

    float triangle_side_a_length(const omath::Triangle<omath::Vector3<float>>& t)
    {
        return t.side_a_length();
    }

    float triangle_side_b_length(const omath::Triangle<omath::Vector3<float>>& t)
    {
        return t.side_b_length();
    }

    omath::Vector3<float> triangle_side_a_vector(const omath::Triangle<omath::Vector3<float>>& t)
    {
        return t.side_a_vector();
    }

    omath::Vector3<float> triangle_side_b_vector(const omath::Triangle<omath::Vector3<float>>& t)
    {
        return t.side_b_vector();
    }

    omath::Vector3<float> triangle_normal(const omath::Triangle<omath::Vector3<float>>& t)
    {
        return t.calculate_normal();
    }

    std::array<float, 4> mat_raw_array_2x2(const omath::Mat<2,2>& m)
    {
        auto arr = m.raw_array();
        return {arr[0], arr[1], arr[2], arr[3]};
    }

    omath::Mat<2,2> mat_transposed_2x2(const omath::Mat<2,2>& m)
    {
        return m.transposed();
    }

    std::optional<omath::Mat<2,2>> mat_inverted_2x2(const omath::Mat<2,2>& m)
    {
        return m.inverted();
    }
}