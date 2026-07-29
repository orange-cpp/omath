#pragma once

#include "omath/lua/lua.hpp"
#include "omath/projection/camera.hpp"
#include "omath/projection/error_codes.hpp"
#include "omath/trigonometry/view_angles.hpp"
#include <sol/sol.hpp>
#include <format>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

namespace omath::lua::detail
{
    using PitchAngle90 = Angle<float, -90.f, 90.f, AngleFlags::Clamped>;
    using PitchAngle89 = Angle<float, -89.f, 89.f, AngleFlags::Clamped>;
    using SharedYawRoll = Angle<float, -180.f, 180.f, AngleFlags::Normalized>;
    using SharedFoV = Angle<float, 0.f, 180.f, AngleFlags::Clamped>;
    using ViewAngles90 = ViewAngles<PitchAngle90, SharedYawRoll, SharedYawRoll>;
    using ViewAngles89 = ViewAngles<PitchAngle89, SharedYawRoll, SharedYawRoll>;

    inline std::string projection_error_to_string(projection::Error error)
    {
        switch (error)
        {
        case projection::Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS:
            return "world position is out of screen bounds";
        case projection::Error::INV_VIEW_PROJ_MAT_DET_EQ_ZERO:
            return "inverse view-projection matrix determinant is zero";
        case projection::Error::PERSPECTIVE_DIVIDER_LESS_EQ_ZERO:
            return "perspective divider is less or equal to zero";
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

    template<class Camera, class PitchAngleType, class ViewAnglesType, class ArithmeticType = float>
    requires std::is_arithmetic_v<ArithmeticType>
    void register_engine(sol::table& omath_table, const char* subtable_name)
    {
        using Mat4X4 = std::remove_cvref_t<decltype(std::declval<const Camera&>().get_view_matrix())>;

        auto engine_table = omath_table[subtable_name].get_or_create<sol::table>();
        auto types = omath_table["_types"].get<sol::table>();

        set_engine_aliases<PitchAngleType, ViewAnglesType>(engine_table, types);

        auto camera_type = engine_table.new_usertype<Camera>(
                "Camera",
                sol::constructors<Camera(const Vector3<ArithmeticType>&, const ViewAnglesType&,
                                         const projection::ViewPort&, const projection::FieldOfView&, ArithmeticType,
                                         ArithmeticType)>());

        camera_type["look_at"] = &Camera::look_at;
        camera_type["get_forward"] = &Camera::get_forward;
        camera_type["get_right"] = &Camera::get_right;
        camera_type["get_up"] = &Camera::get_up;
        camera_type["get_origin"] = &Camera::get_origin;
        camera_type["get_view_angles"] = &Camera::get_view_angles;
        camera_type["get_near_plane"] = &Camera::get_near_plane;
        camera_type["get_far_plane"] = &Camera::get_far_plane;
        camera_type["get_field_of_view"] = &Camera::get_field_of_view;
        camera_type["set_origin"] = &Camera::set_origin;
        camera_type["set_view_angles"] = &Camera::set_view_angles;
        camera_type["set_view_port"] = &Camera::set_view_port;
        camera_type["set_field_of_view"] = &Camera::set_field_of_view;
        camera_type["set_near_plane"] = &Camera::set_near_plane;
        camera_type["set_far_plane"] = &Camera::set_far_plane;
        camera_type["get_view_matrix"] = [](const Camera& camera) -> Mat4X4 { return camera.get_view_matrix(); };
        camera_type["get_projection_matrix"] = [](const Camera& camera) -> Mat4X4
        { return camera.get_projection_matrix(); };
        camera_type["get_view_projection_matrix"] = [](const Camera& camera) -> Mat4X4
        { return camera.get_view_projection_matrix(); };
        camera_type["extract_projection_params"] = [](const Mat4X4& projection_matrix)
        {
            const auto params = Camera::extract_projection_params(projection_matrix);
            return std::make_tuple(params.fov, params.aspect_ratio);
        };
        camera_type["calc_view_angles_from_view_matrix"] = &Camera::calc_view_angles_from_view_matrix;
        camera_type["calc_origin_from_view_matrix"] = &Camera::calc_origin_from_view_matrix;
        camera_type["world_to_screen"] = [](const Camera& camera, const Vector3<ArithmeticType>& position)
                -> std::tuple<sol::optional<Vector3<ArithmeticType>>, sol::optional<std::string>>
        {
            auto result = camera.world_to_screen(position);
            if (result)
                return {*result, sol::nullopt};
            return {sol::nullopt, projection_error_to_string(result.error())};
        };
        camera_type["screen_to_world"] = [](const Camera& camera, const Vector3<ArithmeticType>& position)
                -> std::tuple<sol::optional<Vector3<ArithmeticType>>, sol::optional<std::string>>
        {
            auto result = camera.screen_to_world(position);
            if (result)
                return {*result, sol::nullopt};
            return {sol::nullopt, projection_error_to_string(result.error())};
        };
    }

    void register_opengl_engine(sol::table& omath_table);
    void register_frostbite_engine(sol::table& omath_table);
    void register_iw_engine(sol::table& omath_table);
    void register_source_engine(sol::table& omath_table);
    void register_rage_engine(sol::table& omath_table);
    void register_unity_engine(sol::table& omath_table);
    void register_unreal_engine(sol::table& omath_table);
    void register_cry_engine(sol::table& omath_table);
} // namespace omath::lua::detail
