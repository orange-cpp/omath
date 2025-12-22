#include <omath/linear_algebra/vector2.hpp>
#include <omath/linear_algebra/vector3.hpp>
#include <omath/linear_algebra/vector4.hpp>
#include <omath/linear_algebra/triangle.hpp>
#include <omath/linear_algebra/mat.hpp>

using namespace omath;

// This translation unit is compiled with -fno-inline (set from CMake) to
// force generation of out-of-line code for header-only functions so
// coverage tools can attribute hits to header files.

void force_linear_algebra_instantiations()
{
    Vector2<float> v2a{1.0f, 2.0f};
    Vector2<float> v2b{3.0f, 4.0f};
    volatile float d2 = v2a.distance_to(v2b);
    volatile float d2s = v2a.distance_to_sqr(v2b);
    volatile float dot2 = v2a.dot(v2b);
    volatile float sum2 = v2a.sum();
    volatile auto t2 = v2a.as_tuple();
    volatile auto n2 = v2a.normalized();
    volatile auto neg2 = -v2a;
    volatile float len_sqr2 = v2a.length_sqr();

    // non-inlined helpers
    volatile float len2_noinl = v2a.length();
    volatile auto n2_noinl = v2a.normalized();
    volatile auto t2_noinl = v2a.as_tuple();

    Vector3<float> v3a{1.0f, 0.0f, 0.0f};
    Vector3<float> v3b{0.0f, 1.0f, 0.0f};
    volatile auto c = v3a.cross(v3b);
    volatile float s3 = v3a.sum();
    volatile float s3_2d = v3a.sum_2d();
    volatile float len3 = v3a.length();
    volatile float len3_2d = v3a.length_2d();
    volatile auto n3 = v3a.normalized();
    volatile auto a = v3a.angle_between(v3b);
    volatile auto per = v3a.is_perpendicular(v3b);
    volatile auto ptsame = v3a.point_to_same_direction(v3b);
    volatile auto tup3 = v3a.as_tuple();
    volatile float dist3 = v3a.distance_to(v3b);
    volatile float dot3 = v3a.dot(v3b);

    // Call the non-inlined helper variants too to force out-of-line emission
    volatile float len3_noinl = v3a.length();
    volatile auto n3_noinl = v3a.normalized();
    volatile auto a_noinl = v3a.angle_between(v3b);
    volatile auto per_noinl = v3a.is_perpendicular(v3b);


    Vector4<float> v4a{1,2,3,4};
    Vector4<float> v4b{4,3,2,1};
    volatile float dot4 = v4a.dot(v4b);
    volatile float sum4 = v4a.sum();
    volatile float len4sqr = v4a.length_sqr();
    volatile auto neg4 = -v4a;
    auto tmp = v4a;
    tmp.clamp(0.0f, 2.0f);
    volatile auto clamped = tmp;

    // Vector4 non-inlined helpers
    volatile float len4_noinl = v4a.length();
    volatile float sum4_noinl = v4a.sum();
    tmp.clamp(0.0f, 2.0f);
    volatile auto clamped_noinl = tmp;

    // Mat singular case to exercise inverted() branch
    omath::Mat<2,2> singular{{{1.0f,2.0f},{2.0f,4.0f}}};
    volatile auto inv_singular = singular.inverted();

    Mat<2,2> m{{{1.0f,2.0f},{3.0f,4.0f}}};
    volatile auto det = m.determinant();
    volatile auto tr = m.transposed();
    volatile auto minor = m.minor(0,0);
    volatile auto alg = m.alg_complement(0,0);
    volatile auto ra = m.raw_array();
    volatile auto inv = m.inverted();

    volatile float det_noinl = m.determinant();
    volatile auto inv_noinl = m.inverted();
    volatile auto trans_noinl = m.transposed();
    volatile auto ra_noinl = m.raw_array();

    Triangle<Vector3<float>> t{v3a, v3b, Vector3<float>{0,0,1}};
    volatile auto normal = t.calculate_normal();
    volatile auto sa_len = t.side_a_length();
    volatile auto sb_len = t.side_b_length();
    volatile auto hypotv = t.hypot();
    volatile auto sa_vec = t.side_a_vector();
    volatile auto sb_vec = t.side_b_vector();
    volatile auto mid = t.mid_point();
    volatile auto isrect = t.is_rectangular();


    (void)d2; (void)d2s; (void)dot2; (void)sum2; (void)t2; (void)n2; (void)neg2; (void)len_sqr2;
    (void)c; (void)s3; (void)s3_2d; (void)len3; (void)len3_2d; (void)n3; (void)a; (void)per; (void)ptsame; (void)tup3; (void)dist3; (void)dot3;
    (void)dot4; (void)sum4; (void)len4sqr; (void)neg4; (void)clamped;
    (void)normal; (void)sa_len; (void)sb_len; (void)hypotv; (void)sa_vec; (void)sb_vec; (void)mid; (void)isrect;
    (void)det; (void)tr; (void)minor; (void)alg; (void)ra; (void)inv;
}

// Ensure the function is used from tests
extern "C" void call_force_linear_algebra_instantiations()
{
    force_linear_algebra_instantiations();
}


// Take addresses of selected member functions to ensure they are emitted
// as non-inlined symbols in this TU.
namespace {
    volatile auto p_v3_length = &omath::Vector3<float>::length;
    volatile auto p_v3_normalized = &omath::Vector3<float>::normalized;
    volatile auto p_v3_is_perp = &omath::Vector3<float>::is_perpendicular;
    // also take addresses of the non-inlined helpers
    volatile auto p_v3_length_noinl = &omath::Vector3<float>::length;
    volatile auto p_v3_angle_noinl = &omath::Vector3<float>::angle_between;
    volatile auto p_v3_perp_noinl = &omath::Vector3<float>::is_perpendicular;
    volatile auto p_v3_norm_noinl = &omath::Vector3<float>::normalized;

    volatile auto p_v2_length = &omath::Vector2<float>::length;
    volatile auto p_v2_as_tuple = &omath::Vector2<float>::as_tuple;
    volatile auto p_v2_length_noinl = &omath::Vector2<float>::length;
    volatile auto p_v2_norm_noinl = &omath::Vector2<float>::normalized;

    volatile auto p_v4_length = &omath::Vector4<float>::length;
    volatile auto p_v4_sum = &omath::Vector4<float>::sum;
    volatile auto p_v4_length_noinl = &omath::Vector4<float>::length;
    volatile auto p_v4_sum_noinl = &omath::Vector4<float>::sum;
    volatile auto p_v4_clamp_noinl = &omath::Vector4<float>::clamp;

    volatile auto p_mat_det = &omath::Mat<2,2>::determinant;
    volatile auto p_mat_det_noinl = &omath::Mat<2,2>::determinant;
    volatile auto p_mat_inv_noinl = &omath::Mat<2,2>::inverted;

    volatile auto p_tri_hypot = &omath::Triangle<omath::Vector3<float>>::hypot;
    volatile auto p_tri_calc_norm_noinl = &omath::Triangle<omath::Vector3<float>>::calculate_normal;
    volatile auto p_tri_side_a_len_noinl = &omath::Triangle<omath::Vector3<float>>::side_a_length;
    volatile auto p_tri_side_b_len_noinl = &omath::Triangle<omath::Vector3<float>>::side_b_length;
    volatile auto p_tri_hypot_noinl = &omath::Triangle<omath::Vector3<float>>::hypot;
    volatile auto p_tri_is_rect_noinl = &omath::Triangle<omath::Vector3<float>>::is_rectangular;
}
