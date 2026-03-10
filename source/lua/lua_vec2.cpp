//
// Created by orange on 07.03.2026.
//
#ifdef OMATH_ENABLE_LUA
#include "internal.hpp"
#include <omath/linear_algebra/vector2.hpp>

namespace omath::lua::detail
{
    void register_vec2(sol::table& omath_table)
    {
        using Vec2f = omath::Vector2<float>;

        omath_table.new_usertype<Vec2f>(
                "Vec2", sol::constructors<Vec2f(), Vec2f(float, float)>(),

                "x", &Vec2f::x, "y", &Vec2f::y,

                sol::meta_function::addition, sol::resolve<Vec2f(const Vec2f&) const>(&Vec2f::operator+),
                sol::meta_function::subtraction, sol::resolve<Vec2f(const Vec2f&) const>(&Vec2f::operator-),
                sol::meta_function::unary_minus, sol::resolve<Vec2f() const>(&Vec2f::operator-),
                sol::meta_function::equal_to, &Vec2f::operator==,
                sol::meta_function::less_than, sol::resolve<bool(const Vec2f&) const>(&Vec2f::operator<),
                sol::meta_function::less_than_or_equal_to, sol::resolve<bool(const Vec2f&) const>(&Vec2f::operator<=),
                sol::meta_function::to_string,
                [](const Vec2f& v) { return std::format("Vec2({}, {})", v.x, v.y); },

                sol::meta_function::multiplication,
                sol::overload(sol::resolve<Vec2f(const float&) const>(&Vec2f::operator*),
                              [](const float s, const Vec2f& v) { return v * s; }),

                sol::meta_function::division,
                sol::resolve<Vec2f(const float&) const>(&Vec2f::operator/),

                "length", &Vec2f::length,
                "length_sqr", &Vec2f::length_sqr,
                "normalized", &Vec2f::normalized,
                "dot", &Vec2f::dot,
                "distance_to", &Vec2f::distance_to,
                "distance_to_sqr", &Vec2f::distance_to_sqr,
                "sum", &Vec2f::sum,

                "abs",
                [](const Vec2f& v)
                {
                    Vec2f copy = v;
                    copy.abs();
                    return copy;
                });
    }
} // namespace omath::lua::detail
#endif
