//
// Created by orange on 07.03.2026.
//
#ifdef OMATH_ENABLE_LUA
#include "internal.hpp"
#include <omath/linear_algebra/vector3.hpp>

namespace omath::lua::detail
{
    void register_vec3(sol::table& omath_table)
    {
        using Vec3f = omath::Vector3<float>;

        omath_table.new_usertype<Vec3f>(
                "Vec3", sol::constructors<Vec3f(), Vec3f(float, float, float)>(),

                "x", &Vec3f::x, "y", &Vec3f::y, "z", &Vec3f::z,

                sol::meta_function::addition, sol::resolve<Vec3f(const Vec3f&) const>(&Vec3f::operator+),
                sol::meta_function::subtraction, sol::resolve<Vec3f(const Vec3f&) const>(&Vec3f::operator-),
                sol::meta_function::unary_minus, sol::resolve<Vec3f() const>(&Vec3f::operator-),
                sol::meta_function::equal_to, &Vec3f::operator==, sol::meta_function::less_than,
                sol::resolve<bool(const Vec3f&) const>(&Vec3f::operator<), sol::meta_function::less_than_or_equal_to,
                sol::resolve<bool(const Vec3f&) const>(&Vec3f::operator<=), sol::meta_function::to_string,
                [](const Vec3f& v) { return std::format("Vec3({}, {}, {})", v.x, v.y, v.z); },

                sol::meta_function::multiplication,
                sol::overload(sol::resolve<Vec3f(const float&) const>(&Vec3f::operator*),
                              sol::resolve<Vec3f(const Vec3f&) const>(&Vec3f::operator*),
                              [](const float s, const Vec3f& v) { return v * s; }),

                sol::meta_function::division,
                sol::overload(sol::resolve<Vec3f(const float&) const>(&Vec3f::operator/),
                              sol::resolve<Vec3f(const Vec3f&) const>(&Vec3f::operator/)),

                "length", &Vec3f::length, "length_2d", &Vec3f::length_2d, "length_sqr", &Vec3f::length_sqr,
                "normalized", &Vec3f::normalized, "dot", &Vec3f::dot, "cross", &Vec3f::cross, "distance_to",
                &Vec3f::distance_to, "distance_to_sqr", &Vec3f::distance_to_sqr, "sum",
                sol::resolve<float() const>(&Vec3f::sum), "sum_2d", &Vec3f::sum_2d, "point_to_same_direction",
                &Vec3f::point_to_same_direction, "as_array", &Vec3f::as_array,

                "abs",
                [](const Vec3f& v)
                {
                    Vec3f copy = v;
                    copy.abs();
                    return copy;
                },

                "angle_between",
                [](const Vec3f& self,
                   const Vec3f& other) -> std::tuple<sol::optional<float>, sol::optional<std::string>>
                {
                    auto result = self.angle_between(other);
                    if (result)
                        return std::make_tuple(sol::optional<float>(result->as_degrees()),
                                               sol::optional<std::string>(sol::nullopt));
                    return std::make_tuple(sol::optional<float>(sol::nullopt),
                                           sol::optional<std::string>("impossible angle (zero-length vector)"));
                },

                "is_perpendicular",
                [](const Vec3f& self, const Vec3f& other, sol::optional<float> eps)
                { return self.is_perpendicular(other, eps.value_or(0.0001f)); },

                "as_table",
                [](const Vec3f& v, sol::this_state s) -> sol::table
                {
                    sol::state_view lua(s);
                    sol::table t = lua.create_table();
                    t["x"] = v.x;
                    t["y"] = v.y;
                    t["z"] = v.z;
                    return t;
                });
    }
} // namespace omath::lua::detail
#endif
