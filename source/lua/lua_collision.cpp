//
// Created by orange on 07.03.2026.
//
#ifdef OMATH_ENABLE_LUA
#include "omath/lua/lua.hpp"
#include <omath/3d_primitives/aabb.hpp>
#include <omath/3d_primitives/obb.hpp>
#include <omath/collision/collider_interface.hpp>
#include <omath/collision/epa_algorithm.hpp>
#include <omath/collision/gjk_algorithm.hpp>
#include <omath/collision/line_tracer.hpp>
#include <omath/linear_algebra/triangle.hpp>
#include <omath/linear_algebra/vector3.hpp>
#include <sol/sol.hpp>
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace
{
    using Vec3f = omath::Vector3<float>;
    using Triangle3f = omath::Triangle<Vec3f>;
    using Ray3f = omath::collision::Ray<Vec3f>;
    using LineTracer3f = omath::collision::LineTracer<Ray3f>;
    using Aabbf = omath::primitives::Aabb<float>;
    using Obbf = omath::primitives::Obb<float>;

    template<class Object, class Value>
    auto lua_field(Value Object::* member)
    {
        return sol::property(
                [member](const Object& object) -> const Value&
                {
                    return object.*member;
                },
                [member](Object& object, const Value& value)
                {
                    object.*member = value;
                });
    }

    class LuaConvexCollider final : public omath::collision::ColliderInterface<Vec3f>
    {
    public:
        using VectorType = Vec3f;

        explicit LuaConvexCollider(std::vector<Vec3f> vertices, const Vec3f& origin = {})
            : m_vertices(std::move(vertices)), m_origin(origin)
        {
            if (m_vertices.empty())
                throw std::invalid_argument("convex collider must contain at least one vertex");
        }

        [[nodiscard]]
        Vec3f find_abs_furthest_vertex_position(const Vec3f& direction) const override
        {
            const auto furthest = std::ranges::max_element(
                    m_vertices,
                    [&direction](const Vec3f& first, const Vec3f& second)
                    {
                        return first.dot(direction) < second.dot(direction);
                    });

            return m_origin + *furthest;
        }

        [[nodiscard]]
        const Vec3f& get_origin() const override
        {
            return m_origin;
        }

        void set_origin(const Vec3f& new_origin) override
        {
            m_origin = new_origin;
        }

        [[nodiscard]]
        std::size_t vertex_count() const noexcept
        {
            return m_vertices.size();
        }

        [[nodiscard]]
        const std::vector<Vec3f>& vertices() const noexcept
        {
            return m_vertices;
        }

    private:
        std::vector<Vec3f> m_vertices;
        Vec3f m_origin;
    };

    std::vector<Vec3f> vec3_table_to_vector(const sol::table& points)
    {
        std::vector<Vec3f> result;
        for (std::size_t i = 1;; ++i)
        {
            const auto point = points[i].get<sol::optional<Vec3f>>();
            if (!point)
                break;
            result.push_back(*point);
        }
        return result;
    }

    sol::table vec3_array_to_table(const auto& points, sol::this_state state)
    {
        sol::state_view lua(state);
        sol::table result = lua.create_table(static_cast<int>(points.size()), 0);
        for (std::size_t i = 0; i < points.size(); ++i)
            result[i + 1] = points[i];
        return result;
    }

    Vec3f aabb_top(const Aabbf& aabb, const omath::primitives::UpAxis axis)
    {
        switch (axis)
        {
        case omath::primitives::UpAxis::X:
            return omath::primitives::Aabb<float, omath::primitives::UpAxis::X>{aabb.min, aabb.max}.top();
        case omath::primitives::UpAxis::Y:
            return aabb.top();
        case omath::primitives::UpAxis::Z:
            return omath::primitives::Aabb<float, omath::primitives::UpAxis::Z>{aabb.min, aabb.max}.top();
        }
        std::unreachable();
    }

    Vec3f aabb_bottom(const Aabbf& aabb, const omath::primitives::UpAxis axis)
    {
        switch (axis)
        {
        case omath::primitives::UpAxis::X:
            return omath::primitives::Aabb<float, omath::primitives::UpAxis::X>{aabb.min, aabb.max}.bottom();
        case omath::primitives::UpAxis::Y:
            return aabb.bottom();
        case omath::primitives::UpAxis::Z:
            return omath::primitives::Aabb<float, omath::primitives::UpAxis::Z>{aabb.min, aabb.max}.bottom();
        }
        std::unreachable();
    }

    bool ray_hits_triangle(const Ray3f& ray, const Triangle3f& triangle)
    {
        return !(LineTracer3f::get_ray_hit_point(ray, triangle) == ray.end);
    }

    bool ray_hits_aabb(const Ray3f& ray, const Aabbf& aabb)
    {
        return !(LineTracer3f::get_ray_hit_point(ray, aabb) == ray.end);
    }

    bool ray_hits_obb(const Ray3f& ray, const Obbf& obb)
    {
        return !(LineTracer3f::get_ray_hit_point(ray, obb) == ray.end);
    }
} // namespace

namespace omath::lua
{
    void LuaInterpreter::register_3d_primitives(sol::table& omath_table)
    {
        auto primitives_table = omath_table["primitives"].get_or_create<sol::table>();

        primitives_table.new_enum("UpAxis", "X", omath::primitives::UpAxis::X, "Y", omath::primitives::UpAxis::Y, "Z",
                                  omath::primitives::UpAxis::Z);

        primitives_table.new_usertype<Aabbf>(
                "Aabb",
                sol::factories([]() { return Aabbf{}; },
                               [](const Vec3f& min, const Vec3f& max) { return Aabbf{min, max}; }),

                "min", lua_field(&Aabbf::min), "max", lua_field(&Aabbf::max), "center", &Aabbf::center, "extents",
                &Aabbf::extents,
                "top",
                [](const Aabbf& aabb, sol::optional<omath::primitives::UpAxis> axis)
                {
                    return aabb_top(aabb, axis.value_or(omath::primitives::UpAxis::Y));
                },
                "bottom",
                [](const Aabbf& aabb, sol::optional<omath::primitives::UpAxis> axis)
                {
                    return aabb_bottom(aabb, axis.value_or(omath::primitives::UpAxis::Y));
                },
                "is_collide", &Aabbf::is_collide,
                "as_table",
                [](const Aabbf& aabb, sol::this_state state) -> sol::table
                {
                    sol::state_view lua(state);
                    sol::table result = lua.create_table();
                    result["min"] = aabb.min;
                    result["max"] = aabb.max;
                    return result;
                });

        primitives_table.new_usertype<Obbf>(
                "Obb",
                sol::factories(
                        []()
                        {
                            return Obbf{};
                        },
                        [](const Vec3f& center, const Vec3f& axis_x, const Vec3f& axis_y, const Vec3f& axis_z,
                           const Vec3f& half_extents)
                        {
                            return Obbf{center, axis_x, axis_y, axis_z, half_extents};
                        }),

                "center", lua_field(&Obbf::center), "axis_x", lua_field(&Obbf::axis_x), "axis_y",
                lua_field(&Obbf::axis_y), "axis_z", lua_field(&Obbf::axis_z), "half_extents",
                lua_field(&Obbf::half_extents),
                "vertices",
                [](const Obbf& obb, sol::this_state state)
                {
                    return vec3_array_to_table(obb.vertices(), state);
                });
    }

    void LuaInterpreter::register_collision(sol::table& omath_table)
    {
        auto collision_table = omath_table["collision"].get_or_create<sol::table>();

        collision_table.new_usertype<Ray3f>(
                "Ray",
                sol::factories([]() { return Ray3f{}; },
                               [](const Vec3f& start, const Vec3f& end) { return Ray3f{start, end}; },
                               [](const Vec3f& start, const Vec3f& end, const bool infinite_length)
                               { return Ray3f{start, end, infinite_length}; }),

                "start", lua_field(&Ray3f::start), "end", lua_field(&Ray3f::end), "infinite_length",
                lua_field(&Ray3f::infinite_length),
                "direction_vector", &Ray3f::direction_vector, "direction_vector_normalized",
                &Ray3f::direction_vector_normalized);

        collision_table.new_usertype<LuaConvexCollider>(
                "ConvexCollider",
                sol::factories([](const sol::table& vertices) { return LuaConvexCollider(vec3_table_to_vector(vertices)); },
                               [](const sol::table& vertices, const Vec3f& origin)
                               { return LuaConvexCollider(vec3_table_to_vector(vertices), origin); }),

                "find_abs_furthest_vertex_position", &LuaConvexCollider::find_abs_furthest_vertex_position,
                "get_origin", &LuaConvexCollider::get_origin, "set_origin", &LuaConvexCollider::set_origin,
                "vertex_count", &LuaConvexCollider::vertex_count,
                "vertices",
                [](const LuaConvexCollider& collider, sol::this_state state)
                {
                    return vec3_array_to_table(collider.vertices(), state);
                });

        collision_table.new_usertype<LineTracer3f>(
                "LineTracer", sol::no_constructor, "can_trace_line", &LineTracer3f::can_trace_line,
                "get_ray_hit_point",
                sol::overload(
                        [](const Ray3f& ray, const Triangle3f& triangle)
                        {
                            return LineTracer3f::get_ray_hit_point(ray, triangle);
                        },
                        [](const Ray3f& ray, const Aabbf& aabb)
                        {
                            return LineTracer3f::get_ray_hit_point(ray, aabb);
                        },
                        [](const Ray3f& ray, const Obbf& obb)
                        {
                            return LineTracer3f::get_ray_hit_point(ray, obb);
                        }),
                "ray_hits_triangle", &ray_hits_triangle, "ray_hits_aabb", &ray_hits_aabb, "ray_hits_obb",
                &ray_hits_obb);

        collision_table["gjk_support_vertex"] =
                [](const LuaConvexCollider& collider_a, const LuaConvexCollider& collider_b, const Vec3f& direction)
        { return omath::collision::GjkAlgorithm<LuaConvexCollider>::find_support_vertex(collider_a, collider_b, direction); };

        collision_table["gjk_collide"] = [](const LuaConvexCollider& collider_a, const LuaConvexCollider& collider_b)
        { return omath::collision::GjkAlgorithm<LuaConvexCollider>::is_collide(collider_a, collider_b); };

        collision_table["epa_solve"] = [](const LuaConvexCollider& collider_a, const LuaConvexCollider& collider_b,
                                          sol::this_state state) -> sol::object
        {
            using Gjk = omath::collision::GjkAlgorithm<LuaConvexCollider>;
            using Epa = omath::collision::Epa<LuaConvexCollider>;

            sol::state_view lua(state);
            const auto gjk = Gjk::is_collide_with_simplex_info(collider_a, collider_b);
            if (!gjk.hit)
                return sol::nil;

            const auto epa = Epa::solve(collider_a, collider_b, gjk.simplex);
            if (!epa)
                return sol::nil;

            sol::table result = lua.create_table();
            result["normal"] = epa->normal;
            result["penetration_vector"] = epa->penetration_vector;
            result["depth"] = epa->depth;
            result["iterations"] = epa->iterations;
            result["num_vertices"] = epa->num_vertices;
            result["num_faces"] = epa->num_faces;
            return sol::make_object(lua, result);
        };
    }
} // namespace omath::lua
#endif
