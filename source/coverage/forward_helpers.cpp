#include "coverage_wrappers.hpp"
#include <omath/linear_algebra/vector3.hpp>
#include <omath/linear_algebra/triangle.hpp>
#include <omath/linear_algebra/vector4.hpp>

namespace coverage_wrappers
{
    void call_vector3_forwarders()
    {
        omath::Vector3<float> a{1.f,2.f,3.f};
        omath::Vector3<float> b{4.f,5.f,6.f};

        volatile auto l = a.length(); (void)l;
        volatile auto ang = a.angle_between(b); (void)ang;
        volatile auto per = a.is_perpendicular(b); (void)per;
        volatile auto norm = a.normalized(); (void)norm;
    }

    void call_triangle_forwarders()
    {
        omath::Vector3<float> v1{0.f,0.f,0.f};
        omath::Vector3<float> v2{3.f,0.f,0.f};
        omath::Vector3<float> v3{3.f,4.f,0.f};
        omath::Triangle<omath::Vector3<float>> t{v1,v2,v3};

        volatile auto n = t.calculate_normal(); (void)n;
        volatile auto a = t.side_a_length(); (void)a;
        volatile auto b = t.side_b_length(); (void)b;
        volatile auto h = t.hypot(); (void)h;
        volatile auto r = t.is_rectangular(); (void)r;
    }

    void call_vector4_forwarders()
    {
        omath::Vector4<float> v{1.f,2.f,3.f,4.f};
        volatile auto l = v.length(); (void)l;
        volatile auto s = v.sum(); (void)s;
        v.clamp(-10.f, 10.f);
    }
}
