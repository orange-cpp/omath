//
// Created by orange on 07.03.2026.
//
#ifdef OMATH_ENABLE_LUA
#include "internal.hpp"
#include <omath/linear_algebra/vector4.hpp>

namespace omath::lua::detail
{
    void register_vec4(sol::table& omath_table)
    {
        using Vec4f = omath::Vector4<float>;

        omath_table.new_usertype<Vec4f>(
                "Vec4", sol::constructors<Vec4f(), Vec4f(float, float, float, float)>(),

                "x", sol::property([](const Vec4f& v) { return v.x; }, [](Vec4f& v, float val) { v.x = val; }),
                "y", sol::property([](const Vec4f& v) { return v.y; }, [](Vec4f& v, float val) { v.y = val; }),
                "z", sol::property([](const Vec4f& v) { return v.z; }, [](Vec4f& v, float val) { v.z = val; }),
                "w", sol::property([](const Vec4f& v) { return v.w; }, [](Vec4f& v, float val) { v.w = val; }),

                sol::meta_function::addition, sol::resolve<Vec4f(const Vec4f&) const>(&Vec4f::operator+),
                sol::meta_function::subtraction, sol::resolve<Vec4f(const Vec4f&) const>(&Vec4f::operator-),
                sol::meta_function::unary_minus, sol::resolve<Vec4f() const>(&Vec4f::operator-),
                sol::meta_function::equal_to, &Vec4f::operator==,
                sol::meta_function::less_than, sol::resolve<bool(const Vec4f&) const>(&Vec4f::operator<),
                sol::meta_function::less_than_or_equal_to, sol::resolve<bool(const Vec4f&) const>(&Vec4f::operator<=),
                sol::meta_function::to_string,
                [](const Vec4f& v) { return std::format("Vec4({}, {}, {}, {})", v.x, v.y, v.z, v.w); },

                sol::meta_function::multiplication,
                sol::overload(sol::resolve<Vec4f(const float&) const>(&Vec4f::operator*),
                              sol::resolve<Vec4f(const Vec4f&) const>(&Vec4f::operator*),
                              [](const float s, const Vec4f& v) { return v * s; }),

                sol::meta_function::division,
                sol::overload(sol::resolve<Vec4f(const float&) const>(&Vec4f::operator/),
                              sol::resolve<Vec4f(const Vec4f&) const>(&Vec4f::operator/)),

                "length", &Vec4f::length,
                "length_sqr", &Vec4f::length_sqr,
                "dot", &Vec4f::dot,
                "sum", &Vec4f::sum,

                "abs",
                [](const Vec4f& v)
                {
                    Vec4f copy = v;
                    copy.abs();
                    return copy;
                },

                "clamp",
                [](Vec4f& v, float mn, float mx)
                {
                    v.clamp(mn, mx);
                    return v;
                });
    }
} // namespace omath::lua::detail
#endif
