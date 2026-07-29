//
// Created by orange on 07.03.2026.
//
#ifdef OMATH_ENABLE_LUA
#include "lua_engines_detail.hpp"

namespace omath::lua
{
    void LuaInterpreter::register_shared_types(sol::table& omath_table)
    {
        auto types = omath_table["_types"].get_or_create<sol::table>();

        detail::register_angle<detail::PitchAngle90>(types, "PitchAngle90");
        detail::register_angle<detail::PitchAngle89>(types, "PitchAngle89");
        detail::register_angle<detail::SharedYawRoll>(types, "YawRoll");
        detail::register_angle<detail::SharedFoV>(types, "FieldOfView");

        types.new_usertype<projection::ViewPort>(
                "ViewPort",
                sol::factories([](float width, float height) { return projection::ViewPort{width, height}; }),
                "width",
                sol::property([](const projection::ViewPort& view_port) { return view_port.m_width; },
                              [](projection::ViewPort& view_port, float value) { view_port.m_width = value; }),
                "height",
                sol::property([](const projection::ViewPort& view_port) { return view_port.m_height; },
                              [](projection::ViewPort& view_port, float value) { view_port.m_height = value; }),
                "aspect_ratio", &projection::ViewPort::aspect_ratio);

        types.new_usertype<detail::ViewAngles90>(
                "ViewAngles90",
                sol::factories([](detail::PitchAngle90 pitch, detail::SharedYawRoll yaw, detail::SharedYawRoll roll)
                               { return detail::ViewAngles90{pitch, yaw, roll}; }),
                "pitch",
                sol::property([](const detail::ViewAngles90& view_angles) { return view_angles.pitch; },
                              [](detail::ViewAngles90& view_angles, const detail::PitchAngle90& value)
                              { view_angles.pitch = value; }),
                "yaw",
                sol::property([](const detail::ViewAngles90& view_angles) { return view_angles.yaw; },
                              [](detail::ViewAngles90& view_angles, const detail::SharedYawRoll& value)
                              { view_angles.yaw = value; }),
                "roll",
                sol::property([](const detail::ViewAngles90& view_angles) { return view_angles.roll; },
                              [](detail::ViewAngles90& view_angles, const detail::SharedYawRoll& value)
                              { view_angles.roll = value; }));

        types.new_usertype<detail::ViewAngles89>(
                "ViewAngles89",
                sol::factories([](detail::PitchAngle89 pitch, detail::SharedYawRoll yaw, detail::SharedYawRoll roll)
                               { return detail::ViewAngles89{pitch, yaw, roll}; }),
                "pitch",
                sol::property([](const detail::ViewAngles89& view_angles) { return view_angles.pitch; },
                              [](detail::ViewAngles89& view_angles, const detail::PitchAngle89& value)
                              { view_angles.pitch = value; }),
                "yaw",
                sol::property([](const detail::ViewAngles89& view_angles) { return view_angles.yaw; },
                              [](detail::ViewAngles89& view_angles, const detail::SharedYawRoll& value)
                              { view_angles.yaw = value; }),
                "roll",
                sol::property([](const detail::ViewAngles89& view_angles) { return view_angles.roll; },
                              [](detail::ViewAngles89& view_angles, const detail::SharedYawRoll& value)
                              { view_angles.roll = value; }));
    }

    void LuaInterpreter::register_engines(sol::table& omath_table)
    {
        detail::register_opengl_engine(omath_table);
        detail::register_frostbite_engine(omath_table);
        detail::register_iw_engine(omath_table);
        detail::register_source_engine(omath_table);
        detail::register_rage_engine(omath_table);
        detail::register_unity_engine(omath_table);
        detail::register_unreal_engine(omath_table);
        detail::register_cry_engine(omath_table);
    }
} // namespace omath::lua
#endif
