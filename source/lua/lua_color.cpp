//
// Created by orange on 07.03.2026.
//
#ifdef OMATH_ENABLE_LUA
#include "omath/lua/lua.hpp"
#include <sol/sol.hpp>
#include <omath/utility/color.hpp>

namespace omath::lua::detail
{
    void register_color(sol::table& omath_table)
    {
        omath_table.new_usertype<omath::Color>(
                "Color",
                sol::factories([](float r, float g, float b, float a) { return omath::Color(r, g, b, a); },
                               []() { return omath::Color(); }),

                "from_rgba", [](uint8_t r, uint8_t g, uint8_t b, uint8_t a)
                { return omath::Color::from_rgba(r, g, b, a); }, "from_hsv",
                sol::overload([](float h, float s, float v) { return omath::Color::from_hsv(h, s, v); },
                              [](const omath::Hsv& hsv) { return omath::Color::from_hsv(hsv); }),
                "red", []() { return omath::Color::red(); }, "green", []() { return omath::Color::green(); }, "blue",
                []() { return omath::Color::blue(); },

                "r", sol::property([](const omath::Color& c) { return c.value().x; }), "g",
                sol::property([](const omath::Color& c) { return c.value().y; }), "b",
                sol::property([](const omath::Color& c) { return c.value().z; }), "a",
                sol::property([](const omath::Color& c) { return c.value().w; }),

                "to_hsv", &omath::Color::to_hsv, "set_hue", &omath::Color::set_hue, "set_saturation",
                &omath::Color::set_saturation, "set_value", &omath::Color::set_value, "blend", &omath::Color::blend,

                sol::meta_function::to_string, &omath::Color::to_string);

        omath_table.new_usertype<omath::Hsv>(
                "Hsv", sol::constructors<omath::Hsv()>(), "hue",
                sol::property([](const omath::Hsv& h) { return h.hue; }, [](omath::Hsv& h, float val) { h.hue = val; }),
                "saturation",
                sol::property([](const omath::Hsv& h) { return h.saturation; },
                              [](omath::Hsv& h, float val) { h.saturation = val; }),
                "value",
                sol::property([](const omath::Hsv& h) { return h.value; },
                              [](omath::Hsv& h, float val) { h.value = val; }));
    }
} // namespace omath::lua::detail
#endif
