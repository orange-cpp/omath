//
// Created by orange on 10.03.2026.
//
#ifdef OMATH_ENABLE_LUA
#include "omath/lua/lua.hpp"
#include <sol/sol.hpp>
#include <omath/linear_algebra/triangle.hpp>

namespace omath::lua
{
    void LuaInterpreter::register_triangle(sol::table& omath_table)
    {
        using Vec3f = omath::Vector3<float>;
        using Tri3f = omath::Triangle<Vec3f>;

        omath_table.new_usertype<Tri3f>(
                "Triangle", sol::constructors<Tri3f(), Tri3f(const Vec3f&, const Vec3f&, const Vec3f&)>(),

                "vertex1",
                sol::property([](const Tri3f& t) { return t.m_vertex1; },
                              [](Tri3f& t, const Vec3f& v) { t.m_vertex1 = v; }),
                "vertex2",
                sol::property([](const Tri3f& t) { return t.m_vertex2; },
                              [](Tri3f& t, const Vec3f& v) { t.m_vertex2 = v; }),
                "vertex3",
                sol::property([](const Tri3f& t) { return t.m_vertex3; },
                              [](Tri3f& t, const Vec3f& v) { t.m_vertex3 = v; }),

                "calculate_normal", &Tri3f::calculate_normal,
                "side_a_length", &Tri3f::side_a_length,
                "side_b_length", &Tri3f::side_b_length,
                "side_a_vector", &Tri3f::side_a_vector,
                "side_b_vector", &Tri3f::side_b_vector,
                "hypot", &Tri3f::hypot,
                "is_rectangular", &Tri3f::is_rectangular,
                "mid_point", &Tri3f::mid_point,

                sol::meta_function::to_string,
                [](const Tri3f& t)
                {
                    return std::format("Triangle(({}, {}, {}), ({}, {}, {}), ({}, {}, {}))",
                                       t.m_vertex1.x, t.m_vertex1.y, t.m_vertex1.z,
                                       t.m_vertex2.x, t.m_vertex2.y, t.m_vertex2.z,
                                       t.m_vertex3.x, t.m_vertex3.y, t.m_vertex3.z);
                });
    }
} // namespace omath::lua
#endif
