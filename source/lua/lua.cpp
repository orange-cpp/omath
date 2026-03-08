//
// Created by orange on 07.03.2026.
//
#ifdef OMATH_ENABLE_LUA
#include "lua.hpp"
#include "omath/lua/lua.hpp"
#include <omath/engines/cry_engine/camera.hpp>
#include <omath/engines/cry_engine/constants.hpp>
#include <omath/engines/frostbite_engine/camera.hpp>
#include <omath/engines/frostbite_engine/constants.hpp>
#include <omath/engines/iw_engine/camera.hpp>
#include <omath/engines/iw_engine/constants.hpp>
#include <omath/engines/opengl_engine/camera.hpp>
#include <omath/engines/opengl_engine/constants.hpp>
#include <omath/engines/opengl_engine/formulas.hpp>
#include <omath/engines/source_engine/camera.hpp>
#include <omath/engines/source_engine/constants.hpp>
#include <omath/engines/unity_engine/camera.hpp>
#include <omath/engines/unity_engine/constants.hpp>
#include <omath/engines/unreal_engine/camera.hpp>
#include <omath/engines/unreal_engine/constants.hpp>
#include <omath/linear_algebra/vector2.hpp>
#include <omath/linear_algebra/vector3.hpp>
#include <omath/linear_algebra/vector4.hpp>
#include <omath/utility/color.hpp>
#include <omath/projection/camera.hpp>
#include <omath/projection/error_codes.hpp>
#include <sol/sol.hpp>
#include <string_view>
namespace
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

    void register_vec4(sol::table& omath_table)
    {
        using Vec4f = omath::Vector4<float>;

        omath_table.new_usertype<Vec4f>(
                "Vec4", sol::constructors<Vec4f(), Vec4f(float, float, float, float)>(),

                "x", &Vec4f::x, "y", &Vec4f::y, "z", &Vec4f::z, "w", &Vec4f::w,

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
    static std::string projection_error_to_string(omath::projection::Error e)
    {
        switch (e)
        {
        case omath::projection::Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS:
            return "world position is out of screen bounds";
        case omath::projection::Error::INV_VIEW_PROJ_MAT_DET_EQ_ZERO:
            return "inverse view-projection matrix determinant is zero";
        }
        return "unknown error";
    }

    void register_color(sol::table& omath_table)
    {
        omath_table.new_usertype<omath::Color>(
                "Color",
                sol::factories(
                        [](float r, float g, float b, float a) { return omath::Color(r, g, b, a); },
                        []() { return omath::Color(); }),

                "from_rgba", [](uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
                    return omath::Color::from_rgba(r, g, b, a);
                },
                "from_hsv", sol::overload(
                        [](float h, float s, float v) { return omath::Color::from_hsv(h, s, v); },
                        [](const omath::Hsv& hsv) { return omath::Color::from_hsv(hsv); }),
                "red",   []() { return omath::Color::red(); },
                "green", []() { return omath::Color::green(); },
                "blue",  []() { return omath::Color::blue(); },

                "r", sol::property([](const omath::Color& c) { return c.value().x; }),
                "g", sol::property([](const omath::Color& c) { return c.value().y; }),
                "b", sol::property([](const omath::Color& c) { return c.value().z; }),
                "a", sol::property([](const omath::Color& c) { return c.value().w; }),

                "to_hsv",         &omath::Color::to_hsv,
                "set_hue",        &omath::Color::set_hue,
                "set_saturation", &omath::Color::set_saturation,
                "set_value",      &omath::Color::set_value,
                "blend",          &omath::Color::blend,

                sol::meta_function::to_string, &omath::Color::to_string);

        omath_table.new_usertype<omath::Hsv>(
                "Hsv", sol::constructors<omath::Hsv()>(),
                "hue",        &omath::Hsv::hue,
                "saturation", &omath::Hsv::saturation,
                "value",      &omath::Hsv::value);
    }

    template<class AngleType>
    void register_angle(sol::table& table, const char* name)
    {
        table.new_usertype<AngleType>(
                name, sol::no_constructor,
                "from_degrees", &AngleType::from_degrees,
                "from_radians", &AngleType::from_radians,
                "as_degrees",   &AngleType::as_degrees,
                "as_radians",   &AngleType::as_radians,
                "sin", &AngleType::sin,
                "cos", &AngleType::cos,
                "tan", &AngleType::tan,
                "cot", &AngleType::cot,
                sol::meta_function::addition,
                [](const AngleType& a, const AngleType& b)
                { return AngleType::from_degrees(a.as_degrees() + b.as_degrees()); },
                sol::meta_function::subtraction,
                [](const AngleType& a, const AngleType& b)
                { return AngleType::from_degrees(a.as_degrees() - b.as_degrees()); },
                sol::meta_function::unary_minus,
                [](const AngleType& a) { return AngleType::from_degrees(-a.as_degrees()); },
                sol::meta_function::equal_to,
                [](const AngleType& a, const AngleType& b) { return a == b; },
                sol::meta_function::to_string,
                [](const AngleType& a) { return std::format("{}deg", a.as_degrees()); });
    }

    // ---- Canonical shared C++ type aliases ----------------------------------
    // Each unique template instantiation must be registered exactly once.

    using PitchAngle90  = omath::Angle<float, -90.f, 90.f,   omath::AngleFlags::Clamped>;
    using PitchAngle89  = omath::Angle<float, -89.f, 89.f,   omath::AngleFlags::Clamped>;
    using SharedYawRoll = omath::Angle<float, -180.f, 180.f, omath::AngleFlags::Normalized>;
    using SharedFoV     = omath::Angle<float,   0.f, 180.f,  omath::AngleFlags::Clamped>;
    using ViewAngles90  = omath::ViewAngles<PitchAngle90, SharedYawRoll, SharedYawRoll>;
    using ViewAngles89  = omath::ViewAngles<PitchAngle89, SharedYawRoll, SharedYawRoll>;

    // Register every shared C++ type exactly once under omath._types
    void register_shared_types(sol::table& omath_table)
    {
        auto t = omath_table["_types"].get_or_create<sol::table>();

        register_angle<PitchAngle90>(t,  "PitchAngle90");
        register_angle<PitchAngle89>(t,  "PitchAngle89");
        register_angle<SharedYawRoll>(t, "YawRoll");
        register_angle<SharedFoV>(t,     "FieldOfView");

        t.new_usertype<omath::projection::ViewPort>(
                "ViewPort",
                sol::factories([](float w, float h) { return omath::projection::ViewPort{w, h}; }),
                "width",        &omath::projection::ViewPort::m_width,
                "height",       &omath::projection::ViewPort::m_height,
                "aspect_ratio", &omath::projection::ViewPort::aspect_ratio);

        t.new_usertype<ViewAngles90>(
                "ViewAngles90",
                sol::factories([](PitchAngle90 p, SharedYawRoll y, SharedYawRoll r)
                               { return ViewAngles90{p, y, r}; }),
                "pitch", &ViewAngles90::pitch,
                "yaw",   &ViewAngles90::yaw,
                "roll",  &ViewAngles90::roll);

        t.new_usertype<ViewAngles89>(
                "ViewAngles89",
                sol::factories([](PitchAngle89 p, SharedYawRoll y, SharedYawRoll r)
                               { return ViewAngles89{p, y, r}; }),
                "pitch", &ViewAngles89::pitch,
                "yaw",   &ViewAngles89::yaw,
                "roll",  &ViewAngles89::roll);
    }

    // Set aliases in an engine subtable pointing to the already-registered shared types
    template<class PitchAngleType, class ViewAnglesType>
    void set_engine_aliases(sol::table& engine_table, sol::table& types)
    {
        if constexpr (std::is_same_v<PitchAngleType, PitchAngle90>)
            engine_table["PitchAngle"] = types["PitchAngle90"];
        else
            engine_table["PitchAngle"] = types["PitchAngle89"];

        engine_table["YawAngle"]    = types["YawRoll"];
        engine_table["RollAngle"]   = types["YawRoll"];
        engine_table["FieldOfView"] = types["FieldOfView"];
        engine_table["ViewPort"]    = types["ViewPort"];

        if constexpr (std::is_same_v<ViewAnglesType, ViewAngles90>)
            engine_table["ViewAngles"] = types["ViewAngles90"];
        else
            engine_table["ViewAngles"] = types["ViewAngles89"];
    }

    // Register an engine: alias shared types, register unique Camera
    template<class EngineTraits>
    void register_engine(sol::table& omath_table, const char* subtable_name)
    {
        using PitchAngle = typename EngineTraits::PitchAngle;
        using ViewAngles = typename EngineTraits::ViewAngles;
        using Camera     = typename EngineTraits::Camera;

        auto engine_table = omath_table[subtable_name].get_or_create<sol::table>();
        auto types        = omath_table["_types"].get<sol::table>();

        set_engine_aliases<PitchAngle, ViewAngles>(engine_table, types);

        engine_table.new_usertype<Camera>(
                "Camera",
                sol::constructors<Camera(const omath::Vector3<float>&, const ViewAngles&,
                                         const omath::projection::ViewPort&,
                                         const omath::projection::FieldOfView&, float, float)>(),
                "look_at",           &Camera::look_at,
                "get_forward",       &Camera::get_forward,
                "get_right",         &Camera::get_right,
                "get_up",            &Camera::get_up,
                "get_origin",        &Camera::get_origin,
                "get_view_angles",   &Camera::get_view_angles,
                "get_near_plane",    &Camera::get_near_plane,
                "get_far_plane",     &Camera::get_far_plane,
                "get_field_of_view", &Camera::get_field_of_view,
                "set_origin",        &Camera::set_origin,
                "set_view_angles",   &Camera::set_view_angles,
                "set_view_port",     &Camera::set_view_port,
                "set_field_of_view", &Camera::set_field_of_view,
                "set_near_plane",    &Camera::set_near_plane,
                "set_far_plane",     &Camera::set_far_plane,

                "world_to_screen",
                [](const Camera& cam, const omath::Vector3<float>& pos)
                        -> std::tuple<sol::optional<omath::Vector3<float>>, sol::optional<std::string>>
                {
                    auto result = cam.world_to_screen(pos);
                    if (result) return {*result, sol::nullopt};
                    return {sol::nullopt, projection_error_to_string(result.error())};
                },

                "screen_to_world",
                [](const Camera& cam, const omath::Vector3<float>& pos)
                        -> std::tuple<sol::optional<omath::Vector3<float>>, sol::optional<std::string>>
                {
                    auto result = cam.screen_to_world(pos);
                    if (result) return {*result, sol::nullopt};
                    return {sol::nullopt, projection_error_to_string(result.error())};
                });
    }

    // ---- Engine trait structs -----------------------------------------------

    struct OpenGLEngineTraits
    {
        using PitchAngle = omath::opengl_engine::PitchAngle;
        using ViewAngles = omath::opengl_engine::ViewAngles;
        using Camera     = omath::opengl_engine::Camera;
    };
    struct FrostbiteEngineTraits
    {
        using PitchAngle = omath::frostbite_engine::PitchAngle;
        using ViewAngles = omath::frostbite_engine::ViewAngles;
        using Camera     = omath::frostbite_engine::Camera;
    };
    struct IWEngineTraits
    {
        using PitchAngle = omath::iw_engine::PitchAngle;
        using ViewAngles = omath::iw_engine::ViewAngles;
        using Camera     = omath::iw_engine::Camera;
    };
    struct SourceEngineTraits
    {
        using PitchAngle = omath::source_engine::PitchAngle;
        using ViewAngles = omath::source_engine::ViewAngles;
        using Camera     = omath::source_engine::Camera;
    };
    struct UnityEngineTraits
    {
        using PitchAngle = omath::unity_engine::PitchAngle;
        using ViewAngles = omath::unity_engine::ViewAngles;
        using Camera     = omath::unity_engine::Camera;
    };
    struct UnrealEngineTraits
    {
        using PitchAngle = omath::unreal_engine::PitchAngle;
        using ViewAngles = omath::unreal_engine::ViewAngles;
        using Camera     = omath::unreal_engine::Camera;
    };
    struct CryEngineTraits
    {
        using PitchAngle = omath::cry_engine::PitchAngle;
        using ViewAngles = omath::cry_engine::ViewAngles;
        using Camera     = omath::cry_engine::Camera;
    };
} // namespace

namespace omath::lua
{
    void register_lib(lua_State* lua_state)
    {
        sol::state_view lua(lua_state);

        auto omath_table = lua["omath"].get_or_create<sol::table>();

        register_vec2(omath_table);
        register_vec3(omath_table);
        register_vec4(omath_table);
        register_color(omath_table);
        register_shared_types(omath_table);
        register_engine<OpenGLEngineTraits>(omath_table,    "opengl");
        register_engine<FrostbiteEngineTraits>(omath_table, "frostbite");
        register_engine<IWEngineTraits>(omath_table,        "iw");
        register_engine<SourceEngineTraits>(omath_table,    "source");
        register_engine<UnityEngineTraits>(omath_table,     "unity");
        register_engine<UnrealEngineTraits>(omath_table,    "unreal");
        register_engine<CryEngineTraits>(omath_table,       "cry");
    }
} // namespace omath::lua
#endif