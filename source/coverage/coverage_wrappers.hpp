#pragma once

#include <omath/linear_algebra/vector2.hpp>
#include <omath/linear_algebra/vector3.hpp>
#include <omath/linear_algebra/vector4.hpp>
#include <omath/linear_algebra/triangle.hpp>
#include <omath/linear_algebra/mat.hpp>
#include <tuple>

namespace coverage_wrappers
{
    float vector2_distance(const omath::Vector2<float>& a, const omath::Vector2<float>& b);
    float vector2_dot(const omath::Vector2<float>& a, const omath::Vector2<float>& b);
    omath::Vector3<float> vector3_cross(const omath::Vector3<float>& a, const omath::Vector3<float>& b);
    float vector3_angle_between_deg(const omath::Vector3<float>& a, const omath::Vector3<float>& b);
    float vector4_dot(const omath::Vector4<float>& a, const omath::Vector4<float>& b);

    float vector2_normalized_length(const omath::Vector2<float>& a);
    float vector2_abs_sum(const omath::Vector2<float>& a);
    std::tuple<float, float> vector2_as_tuple(const omath::Vector2<float>& a);

    float vector3_length(const omath::Vector3<float>& a);
    float vector3_length_2d(const omath::Vector3<float>& a);
    bool vector3_is_perpendicular(const omath::Vector3<float>& a, const omath::Vector3<float>& b);
    bool vector3_point_to_same_direction(const omath::Vector3<float>& a, const omath::Vector3<float>& b);

    float vector4_clamp_x(const omath::Vector4<float>& a, float minv, float maxv);
    float vector4_abs_sum(const omath::Vector4<float>& a);

    float triangle_hypot(const omath::Triangle<omath::Vector3<float>>& t);
    omath::Vector3<float> triangle_midpoint(const omath::Triangle<omath::Vector3<float>>& t);
    bool triangle_is_rectangular(const omath::Triangle<omath::Vector3<float>>& t);

    float mat_det_2x2(const omath::Mat<2,2>& m);

    omath::Vector2<float> vector2_add(const omath::Vector2<float>& a, const omath::Vector2<float>& b);
    omath::Vector2<float> vector2_sub(const omath::Vector2<float>& a, const omath::Vector2<float>& b);
    omath::Vector2<float> vector2_mul_scalar(const omath::Vector2<float>& a, float s);
    omath::Vector2<float> vector2_div_scalar(const omath::Vector2<float>& a, float s);
    float vector2_length_sqr(const omath::Vector2<float>& a);
    omath::Vector2<float> vector2_negate(const omath::Vector2<float>& a);

    float triangle_side_a_length(const omath::Triangle<omath::Vector3<float>>& t);
    float triangle_side_b_length(const omath::Triangle<omath::Vector3<float>>& t);
    omath::Vector3<float> triangle_side_a_vector(const omath::Triangle<omath::Vector3<float>>& t);
    omath::Vector3<float> triangle_side_b_vector(const omath::Triangle<omath::Vector3<float>>& t);
    omath::Vector3<float> triangle_normal(const omath::Triangle<omath::Vector3<float>>& t);

    std::array<float, 4> mat_raw_array_2x2(const omath::Mat<2,2>& m);
    omath::Mat<2,2> mat_transposed_2x2(const omath::Mat<2,2>& m);
    std::optional<omath::Mat<2,2>> mat_inverted_2x2(const omath::Mat<2,2>& m);

    // Forwarder functions implemented in a compiled TU to call non-template
    // wrappers that invoke header helpers. These help force symbol emission
    // in a non-template translation unit so coverage tools can attribute
    // runtime hits back to header lines.
    void call_vector3_forwarders();
    void call_triangle_forwarders();
    void call_vector4_forwarders();
    
    // Mat initializer-list coverage helpers (namespaced wrappers)
    void coverage_mat_init_rows_mismatch();
    void coverage_mat_init_columns_mismatch();
}

// Backwards-compatible global declarations (callable from tests without namespace)
void coverage_mat_init_rows_mismatch();
void coverage_mat_init_columns_mismatch();
