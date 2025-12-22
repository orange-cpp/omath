// Purpose: small compiled translation unit that calls non-inlined header helpers
// to force emission of template instantiations / symbols with header line info.
#include <omath/linear_algebra/vector3.hpp>
#include <omath/linear_algebra/triangle.hpp>
#include <omath/linear_algebra/vector4.hpp>

extern "C" void call_extra_linear_algebra_coverage()
{
    using Vec3 = omath::Vector3<float>;
    using Vec4 = omath::Vector4<float>;
    using Tri3 = omath::Triangle<Vec3>;

    Vec3 a{1.0f, 0.0f, 0.0f};
    Vec3 b{0.0f, 1.0f, 0.0f};
    Vec3 c{0.0f, 0.0f, 1.0f};

    // Call noinline helpers to force instantiation and emit line tables.
    volatile float la = a.length();
    (void)la;

    auto angle = a.angle_between(b);
    (void)angle;

    volatile bool perp = a.is_perpendicular(b);
    (void)perp;

    Tri3 t{a, b, c};
    volatile auto n = t.calculate_normal();
    (void)n;

    Vec4 v4{1.0f, 2.0f, 3.0f, 4.0f};
    volatile float s = v4.sum();
    (void)s;
}
