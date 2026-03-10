//
// Created by orange on 07.03.2026.
//
#ifdef OMATH_ENABLE_LUA
#include "omath/lua/lua.hpp"
#include <omath/engines/cry_engine/camera.hpp>
#include <omath/engines/frostbite_engine/camera.hpp>
#include <omath/engines/iw_engine/camera.hpp>
#include <omath/engines/opengl_engine/camera.hpp>
#include <omath/engines/source_engine/camera.hpp>
#include <omath/engines/unity_engine/camera.hpp>
#include <omath/engines/unreal_engine/camera.hpp>
#include <sol/sol.hpp>
#include <string_view>

namespace
{
    // ---- Canonical shared C++ type aliases ----------------------------------
    // Each unique template instantiation must be registered exactly once.

    using PitchAngle90 = omath::Angle<float, -90.f, 90.f, omath::AngleFlags::Clamped>;
    using PitchAngle89 = omath::Angle<float, -89.f, 89.f, omath::AngleFlags::Clamped>;
    using SharedYawRoll = omath::Angle<float, -180.f, 180.f, omath::AngleFlags::Normalized>;
    using SharedFoV = omath::Angle<float, 0.f, 180.f, omath::AngleFlags::Clamped>;
    using ViewAngles90 = omath::ViewAngles<PitchAngle90, SharedYawRoll, SharedYawRoll>;
    using ViewAngles89 = omath::ViewAngles<PitchAngle89, SharedYawRoll, SharedYawRoll>;

    std::string projection_error_to_string(omath::projection::Error e)
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

    template<class AngleType>
    void register_angle(sol::table& table, const char* name)
    {
        table.new_usertype<AngleType>(
                name, sol::no_constructor, "from_degrees", &AngleType::from_degrees, "from_radians",
                &AngleType::from_radians, "as_degrees", &AngleType::as_degrees, "as_radians", &AngleType::as_radians,
                "sin", &AngleType::sin, "cos", &AngleType::cos, "tan", &AngleType::tan, "cot", &AngleType::cot,
                sol::meta_function::addition, [](const AngleType& a, const AngleType& b)
                { return AngleType::from_degrees(a.as_degrees() + b.as_degrees()); }, sol::meta_function::subtraction,
                [](const AngleType& a, const AngleType& b)
                { return AngleType::from_degrees(a.as_degrees() - b.as_degrees()); }, sol::meta_function::unary_minus,
                [](const AngleType& a) { return AngleType::from_degrees(-a.as_degrees()); },
                sol::meta_function::equal_to, [](const AngleType& a, const AngleType& b) { return a == b; },
                sol::meta_function::to_string, [](const AngleType& a) { return std::format("{}deg", a.as_degrees()); });
    }

    // Set aliases in an engine subtable pointing to the already-registered shared types
    template<class PitchAngleType, class ViewAnglesType>
    void set_engine_aliases(sol::table& engine_table, sol::table& types)
    {
        if constexpr (std::is_same_v<PitchAngleType, PitchAngle90>)
            engine_table["PitchAngle"] = types["PitchAngle90"];
        else
            engine_table["PitchAngle"] = types["PitchAngle89"];

        engine_table["YawAngle"] = types["YawRoll"];
        engine_table["RollAngle"] = types["YawRoll"];
        engine_table["FieldOfView"] = types["FieldOfView"];
        engine_table["ViewPort"] = types["ViewPort"];

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
        using Camera = typename EngineTraits::Camera;

        auto engine_table = omath_table[subtable_name].get_or_create<sol::table>();
        auto types = omath_table["_types"].get<sol::table>();

        set_engine_aliases<PitchAngle, ViewAngles>(engine_table, types);

        engine_table.new_usertype<Camera>(
                "Camera",
                sol::constructors<Camera(const omath::Vector3<float>&, const ViewAngles&,
                                         const omath::projection::ViewPort&, const omath::projection::FieldOfView&,
                                         float, float)>(),
                "look_at", &Camera::look_at, "get_forward", &Camera::get_forward, "get_right", &Camera::get_right,
                "get_up", &Camera::get_up, "get_origin", &Camera::get_origin, "get_view_angles",
                &Camera::get_view_angles, "get_near_plane", &Camera::get_near_plane, "get_far_plane",
                &Camera::get_far_plane, "get_field_of_view", &Camera::get_field_of_view, "set_origin",
                &Camera::set_origin, "set_view_angles", &Camera::set_view_angles, "set_view_port",
                &Camera::set_view_port, "set_field_of_view", &Camera::set_field_of_view, "set_near_plane",
                &Camera::set_near_plane, "set_far_plane", &Camera::set_far_plane,

                "world_to_screen",
                [](const Camera& cam, const omath::Vector3<float>& pos)
                        -> std::tuple<sol::optional<omath::Vector3<float>>, sol::optional<std::string>>
                {
                    auto result = cam.world_to_screen(pos);
                    if (result)
                        return {*result, sol::nullopt};
                    return {sol::nullopt, projection_error_to_string(result.error())};
                },

                "screen_to_world",
                [](const Camera& cam, const omath::Vector3<float>& pos)
                        -> std::tuple<sol::optional<omath::Vector3<float>>, sol::optional<std::string>>
                {
                    auto result = cam.screen_to_world(pos);
                    if (result)
                        return {*result, sol::nullopt};
                    return {sol::nullopt, projection_error_to_string(result.error())};
                });
    }

    // ---- Engine trait structs -----------------------------------------------

    struct OpenGLEngineTraits
    {
        using PitchAngle = omath::opengl_engine::PitchAngle;
        using ViewAngles = omath::opengl_engine::ViewAngles;
        using Camera = omath::opengl_engine::Camera;
    };
    struct FrostbiteEngineTraits
    {
        using PitchAngle = omath::frostbite_engine::PitchAngle;
        using ViewAngles = omath::frostbite_engine::ViewAngles;
        using Camera = omath::frostbite_engine::Camera;
    };
    struct IWEngineTraits
    {
        using PitchAngle = omath::iw_engine::PitchAngle;
        using ViewAngles = omath::iw_engine::ViewAngles;
        using Camera = omath::iw_engine::Camera;
    };
    struct SourceEngineTraits
    {
        using PitchAngle = omath::source_engine::PitchAngle;
        using ViewAngles = omath::source_engine::ViewAngles;
        using Camera = omath::source_engine::Camera;
    };
    struct UnityEngineTraits
    {
        using PitchAngle = omath::unity_engine::PitchAngle;
        using ViewAngles = omath::unity_engine::ViewAngles;
        using Camera = omath::unity_engine::Camera;
    };
    struct UnrealEngineTraits
    {
        using PitchAngle = omath::unreal_engine::PitchAngle;
        using ViewAngles = omath::unreal_engine::ViewAngles;
        using Camera = omath::unreal_engine::Camera;
    };
    struct CryEngineTraits
    {
        using PitchAngle = omath::cry_engine::PitchAngle;
        using ViewAngles = omath::cry_engine::ViewAngles;
        using Camera = omath::cry_engine::Camera;
    };
} // namespace

namespace omath::lua
{
    void LuaInterpreter::register_shared_types(sol::table& omath_table)
    {
        auto t = omath_table["_types"].get_or_create<sol::table>();

        register_angle<PitchAngle90>(t, "PitchAngle90");
        register_angle<PitchAngle89>(t, "PitchAngle89");
        register_angle<SharedYawRoll>(t, "YawRoll");
        register_angle<SharedFoV>(t, "FieldOfView");

        t.new_usertype<omath::projection::ViewPort>(
                "ViewPort", sol::factories([](float w, float h) { return omath::projection::ViewPort{w, h}; }), "width",
                sol::property([](const omath::projection::ViewPort& vp) { return vp.m_width; },
                              [](omath::projection::ViewPort& vp, float val) { vp.m_width = val; }),
                "height",
                sol::property([](const omath::projection::ViewPort& vp) { return vp.m_height; },
                              [](omath::projection::ViewPort& vp, float val) { vp.m_height = val; }),
                "aspect_ratio", &omath::projection::ViewPort::aspect_ratio);

        t.new_usertype<ViewAngles90>(
                "ViewAngles90",
                sol::factories([](PitchAngle90 p, SharedYawRoll y, SharedYawRoll r) { return ViewAngles90{p, y, r}; }),
                "pitch",
                sol::property([](const ViewAngles90& va) { return va.pitch; },
                              [](ViewAngles90& va, const PitchAngle90& val) { va.pitch = val; }),
                "yaw",
                sol::property([](const ViewAngles90& va) { return va.yaw; },
                              [](ViewAngles90& va, const SharedYawRoll& val) { va.yaw = val; }),
                "roll",
                sol::property([](const ViewAngles90& va) { return va.roll; },
                              [](ViewAngles90& va, const SharedYawRoll& val) { va.roll = val; }));

        t.new_usertype<ViewAngles89>(
                "ViewAngles89",
                sol::factories([](PitchAngle89 p, SharedYawRoll y, SharedYawRoll r) { return ViewAngles89{p, y, r}; }),
                "pitch",
                sol::property([](const ViewAngles89& va) { return va.pitch; },
                              [](ViewAngles89& va, const PitchAngle89& val) { va.pitch = val; }),
                "yaw",
                sol::property([](const ViewAngles89& va) { return va.yaw; },
                              [](ViewAngles89& va, const SharedYawRoll& val) { va.yaw = val; }),
                "roll",
                sol::property([](const ViewAngles89& va) { return va.roll; },
                              [](ViewAngles89& va, const SharedYawRoll& val) { va.roll = val; }));
    }

    void LuaInterpreter::register_engines(sol::table& omath_table)
    {
        register_engine<OpenGLEngineTraits>(omath_table, "opengl");
        register_engine<FrostbiteEngineTraits>(omath_table, "frostbite");
        register_engine<IWEngineTraits>(omath_table, "iw");
        register_engine<SourceEngineTraits>(omath_table, "source");
        register_engine<UnityEngineTraits>(omath_table, "unity");
        register_engine<UnrealEngineTraits>(omath_table, "unreal");
        register_engine<CryEngineTraits>(omath_table, "cry");
    }
} // namespace omath::lua::detail
#endif
