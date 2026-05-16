//
// Created by orange on 07.03.2026.
//
#ifdef OMATH_ENABLE_LUA
#include "omath/lua/lua.hpp"
#include <omath/linear_algebra/quaternion.hpp>
#include <sol/sol.hpp>

namespace omath::lua
{
    void LuaInterpreter::register_quaternion(sol::table& omath_table)
    {
        using Quatf = omath::Quaternion<float>;
        using Vec3f = omath::Vector3<float>;

        omath_table.new_usertype<Quatf>(
                "Quaternion", sol::constructors<Quatf(), Quatf(float, float, float, float)>(),

                "from_axis_angle", &Quatf::from_axis_angle,

                "x",
                sol::property(
                        [](const Quatf& q)
                        {
                            return q.x;
                        },
                        [](Quatf& q, float val)
                        {
                            q.x = val;
                        }),
                "y",
                sol::property(
                        [](const Quatf& q)
                        {
                            return q.y;
                        },
                        [](Quatf& q, float val)
                        {
                            q.y = val;
                        }),
                "z",
                sol::property(
                        [](const Quatf& q)
                        {
                            return q.z;
                        },
                        [](Quatf& q, float val)
                        {
                            q.z = val;
                        }),
                "w",
                sol::property(
                        [](const Quatf& q)
                        {
                            return q.w;
                        },
                        [](Quatf& q, float val)
                        {
                            q.w = val;
                        }),

                sol::meta_function::addition, sol::resolve<Quatf(const Quatf&) const>(&Quatf::operator+),
                sol::meta_function::multiplication,
                sol::overload(sol::resolve<Quatf(const Quatf&) const>(&Quatf::operator*),
                              sol::resolve<Quatf(const float&) const>(&Quatf::operator*),
                              [](float s, const Quatf& q)
                              {
                                  return q * s;
                              }),
                sol::meta_function::unary_minus, sol::resolve<Quatf() const>(&Quatf::operator-),
                sol::meta_function::equal_to, &Quatf::operator==, sol::meta_function::to_string,
                [](const Quatf& q)
                {
                    return std::format("Quaternion({}, {}, {}, {})", q.x, q.y, q.z, q.w);
                },

                "conjugate", &Quatf::conjugate, "dot", &Quatf::dot, "length", &Quatf::length, "length_sqr",
                &Quatf::length_sqr, "normalized", &Quatf::normalized, "inverse", &Quatf::inverse, "rotate",
                sol::resolve<Vec3f(const Vec3f&) const>(&Quatf::rotate), "to_rotation_matrix3",
                &Quatf::to_rotation_matrix3, "to_rotation_matrix4", &Quatf::to_rotation_matrix4,

                "as_table",
                [](const Quatf& q, sol::this_state s) -> sol::table
                {
                    sol::state_view lua(s);
                    sol::table t = lua.create_table();
                    t["x"] = q.x;
                    t["y"] = q.y;
                    t["z"] = q.z;
                    t["w"] = q.w;
                    return t;
                });
    }
} // namespace omath::lua
#endif
