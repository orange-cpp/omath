//
// Created by orange on 07.03.2026.
//
#ifdef OMATH_ENABLE_LUA
#include "omath/lua/lua.hpp"
#include <omath/hud/canvas_box.hpp>
#include <omath/hud/entity_overlay.hpp>
#include <sol/sol.hpp>
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    class LuaHudRenderer final : public omath::hud::HudRendererInterface
    {
    public:
        explicit LuaHudRenderer(sol::table callbacks): m_callbacks(std::move(callbacks))
        {
        }

        void add_line(const omath::Vector2<float>& line_start, const omath::Vector2<float>& line_end,
                      const omath::Color& color, const float thickness) override
        {
            call_optional("add_line", line_start, line_end, color, thickness);
        }

        void add_polyline(const std::span<const omath::Vector2<float>>& vertexes, const omath::Color& color,
                          const float thickness) override
        {
            call_optional("add_polyline", make_points_table(vertexes), color, thickness);
        }

        void add_filled_polyline(const std::span<const omath::Vector2<float>>& vertexes,
                                 const omath::Color& color) override
        {
            call_optional("add_filled_polyline", make_points_table(vertexes), color);
        }

        void add_rectangle(const omath::Vector2<float>& min, const omath::Vector2<float>& max,
                           const omath::Color& color) override
        {
            call_optional("add_rectangle", min, max, color);
        }

        void add_filled_rectangle(const omath::Vector2<float>& min, const omath::Vector2<float>& max,
                                  const omath::Color& color) override
        {
            call_optional("add_filled_rectangle", min, max, color);
        }

        void add_circle(const omath::Vector2<float>& center, const float radius, const omath::Color& color,
                        const float thickness, const int segments) override
        {
            call_optional("add_circle", center, radius, color, thickness, segments);
        }

        void add_filled_circle(const omath::Vector2<float>& center, const float radius, const omath::Color& color,
                               const int segments) override
        {
            call_optional("add_filled_circle", center, radius, color, segments);
        }

        void add_arc(const omath::Vector2<float>& center, const float radius, const float a_min, const float a_max,
                     const omath::Color& color, const float thickness, const int segments) override
        {
            call_optional("add_arc", center, radius, a_min, a_max, color, thickness, segments);
        }

        void add_image(const std::any& texture_id, const omath::Vector2<float>& min, const omath::Vector2<float>& max,
                       const omath::Color& tint) override
        {
            const auto callback = callback_for("add_image");
            if (!callback)
                return;

            sol::object texture = sol::nil;
            if (const auto lua_object = std::any_cast<sol::object>(&texture_id))
                texture = *lua_object;

            auto result = (*callback)(texture, min, max, tint);
            if (!result.valid())
                throw sol::error(result);
        }

        void add_text(const omath::Vector2<float>& position, const omath::Color& color,
                      const std::string_view& text) override
        {
            call_optional("add_text", position, color, std::string{text});
        }

        [[nodiscard]]
        omath::Vector2<float> calc_text_size(const std::string_view& text) override
        {
            const auto callback = callback_for("calc_text_size");
            if (!callback)
                return {};

            auto result = (*callback)(std::string{text});
            if (!result.valid())
                throw sol::error(result);

            return result.get<omath::Vector2<float>>();
        }

    private:
        sol::main_table m_callbacks;

        [[nodiscard]]
        sol::optional<sol::protected_function> callback_for(const char* name) const
        {
            const sol::object callback = m_callbacks[name];
            if (!callback.valid() || callback == sol::nil)
                return sol::nullopt;
            return callback.as<sol::protected_function>();
        }

        template<class... Args>
        void call_optional(const char* name, Args&&... args) const
        {
            const auto callback = callback_for(name);
            if (!callback)
                return;

            auto result = (*callback)(std::forward<Args>(args)...);
            if (!result.valid())
                throw sol::error(result);
        }

        [[nodiscard]]
        sol::table make_points_table(const std::span<const omath::Vector2<float>>& vertexes) const
        {
            sol::state_view lua(m_callbacks.lua_state());
            sol::table points = lua.create_table(static_cast<int>(vertexes.size()), 0);
            for (std::size_t i = 0; i < vertexes.size(); ++i)
                points[i + 1] = vertexes[i];
            return points;
        }
    };

    [[nodiscard]]
    omath::Color transparent_black()
    {
        return {0.f, 0.f, 0.f, 0.f};
    }

    [[nodiscard]]
    omath::Color opaque_white()
    {
        return {1.f, 1.f, 1.f, 1.f};
    }

    [[nodiscard]]
    omath::hud::EntityOverlay make_overlay(const omath::Vector2<float>& top, const omath::Vector2<float>& bottom,
                                           const std::shared_ptr<LuaHudRenderer>& renderer)
    {
        if (!renderer)
            throw std::invalid_argument("hud renderer must not be nil");
        return {top, bottom, renderer};
    }

    [[nodiscard]]
    std::any texture_id_from_lua_object(const sol::object& texture_id)
    {
        return texture_id;
    }

    std::vector<omath::Vector2<float>> points_from_table(const sol::table& points)
    {
        std::vector<omath::Vector2<float>> result;
        for (std::size_t i = 1;; ++i)
        {
            const auto point = points[i].get<sol::optional<omath::Vector2<float>>>();
            if (!point)
                break;
            result.push_back(*point);
        }
        return result;
    }

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
} // namespace

namespace omath::lua
{
    void LuaInterpreter::register_hud(sol::table& omath_table)
    {
        auto hud_table = omath_table["hud"].get_or_create<sol::table>();

        hud_table.new_usertype<omath::hud::CanvasBox>(
                "CanvasBox",
                sol::factories([](const omath::Vector2<float>& top, const omath::Vector2<float>& bottom)
                               { return omath::hud::CanvasBox(top, bottom); },
                               [](const omath::Vector2<float>& top, const omath::Vector2<float>& bottom,
                                  const float ratio) { return omath::hud::CanvasBox(top, bottom, ratio); }),

                "top_left_corner", lua_field(&omath::hud::CanvasBox::top_left_corner), "top_right_corner",
                lua_field(&omath::hud::CanvasBox::top_right_corner), "bottom_left_corner",
                lua_field(&omath::hud::CanvasBox::bottom_left_corner), "bottom_right_corner",
                lua_field(&omath::hud::CanvasBox::bottom_right_corner),

                "as_table",
                [](const omath::hud::CanvasBox& box, sol::this_state s) -> sol::table
                {
                    sol::state_view lua(s);
                    sol::table t = lua.create_table(4, 0);
                    const auto points = box.as_array();
                    for (std::size_t i = 0; i < points.size(); ++i)
                        t[i + 1] = points[i];
                    return t;
                });

        hud_table.new_usertype<LuaHudRenderer>(
                "Renderer",
                sol::factories([](const sol::table& callbacks)
                               { return std::make_shared<LuaHudRenderer>(callbacks); }),

                "add_line", &LuaHudRenderer::add_line,
                "add_polyline",
                [](LuaHudRenderer& renderer, const sol::table& points, const omath::Color& color,
                   const float thickness)
                {
                    const auto vertices = points_from_table(points);
                    renderer.add_polyline({vertices.data(), vertices.size()}, color, thickness);
                },
                "add_filled_polyline",
                [](LuaHudRenderer& renderer, const sol::table& points, const omath::Color& color)
                {
                    const auto vertices = points_from_table(points);
                    renderer.add_filled_polyline({vertices.data(), vertices.size()}, color);
                },
                "add_rectangle", &LuaHudRenderer::add_rectangle, "add_filled_rectangle",
                &LuaHudRenderer::add_filled_rectangle, "add_circle",
                [](LuaHudRenderer& renderer, const omath::Vector2<float>& center, const float radius,
                   const omath::Color& color, const float thickness, sol::optional<int> segments)
                {
                    renderer.add_circle(center, radius, color, thickness, segments.value_or(0));
                },
                "add_filled_circle",
                [](LuaHudRenderer& renderer, const omath::Vector2<float>& center, const float radius,
                   const omath::Color& color, sol::optional<int> segments)
                {
                    renderer.add_filled_circle(center, radius, color, segments.value_or(0));
                },
                "add_arc",
                [](LuaHudRenderer& renderer, const omath::Vector2<float>& center, const float radius,
                   const float a_min, const float a_max, const omath::Color& color, const float thickness,
                   sol::optional<int> segments)
                {
                    renderer.add_arc(center, radius, a_min, a_max, color, thickness, segments.value_or(0));
                },
                "add_image",
                [](LuaHudRenderer& renderer, const sol::object& texture_id, const omath::Vector2<float>& min,
                   const omath::Vector2<float>& max, sol::optional<omath::Color> tint)
                {
                    renderer.add_image(texture_id_from_lua_object(texture_id), min, max,
                                       tint.value_or(opaque_white()));
                },
                "add_text", &LuaHudRenderer::add_text, "calc_text_size", &LuaHudRenderer::calc_text_size);

        hud_table.new_usertype<omath::hud::EntityOverlay>(
                "EntityOverlay",
                sol::factories(&make_overlay),

                "add_2d_box",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& box_color,
                   sol::optional<omath::Color> fill_color, sol::optional<float> thickness) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_2d_box(box_color, fill_color.value_or(transparent_black()),
                                              thickness.value_or(1.f));
                },
                "add_cornered_2d_box",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& box_color,
                   sol::optional<omath::Color> fill_color, sol::optional<float> corner_ratio_len,
                   sol::optional<float> thickness) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_cornered_2d_box(box_color, fill_color.value_or(transparent_black()),
                                                       corner_ratio_len.value_or(0.2f), thickness.value_or(1.f));
                },
                "add_dashed_box",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, sol::optional<float> dash_len,
                   sol::optional<float> gap_len, sol::optional<float> thickness) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_dashed_box(color, dash_len.value_or(8.f), gap_len.value_or(5.f),
                                                  thickness.value_or(1.f));
                },

                "add_right_bar",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const omath::Color& outline_color,
                   const omath::Color& bg_color, const float width, const float ratio,
                   sol::optional<float> offset) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_right_bar(color, outline_color, bg_color, width, ratio, offset.value_or(5.f));
                },
                "add_left_bar",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const omath::Color& outline_color,
                   const omath::Color& bg_color, const float width, const float ratio,
                   sol::optional<float> offset) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_left_bar(color, outline_color, bg_color, width, ratio, offset.value_or(5.f));
                },
                "add_top_bar",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const omath::Color& outline_color,
                   const omath::Color& bg_color, const float height, const float ratio,
                   sol::optional<float> offset) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_top_bar(color, outline_color, bg_color, height, ratio, offset.value_or(5.f));
                },
                "add_bottom_bar",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const omath::Color& outline_color,
                   const omath::Color& bg_color, const float height, const float ratio,
                   sol::optional<float> offset) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_bottom_bar(color, outline_color, bg_color, height, ratio, offset.value_or(5.f));
                },
                "add_right_dashed_bar",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const omath::Color& outline_color,
                   const omath::Color& bg_color, const float width, const float ratio, const float dash_len,
                   const float gap_len, sol::optional<float> offset) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_right_dashed_bar(color, outline_color, bg_color, width, ratio, dash_len, gap_len,
                                                        offset.value_or(5.f));
                },
                "add_left_dashed_bar",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const omath::Color& outline_color,
                   const omath::Color& bg_color, const float width, const float ratio, const float dash_len,
                   const float gap_len, sol::optional<float> offset) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_left_dashed_bar(color, outline_color, bg_color, width, ratio, dash_len, gap_len,
                                                       offset.value_or(5.f));
                },
                "add_top_dashed_bar",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const omath::Color& outline_color,
                   const omath::Color& bg_color, const float height, const float ratio, const float dash_len,
                   const float gap_len, sol::optional<float> offset) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_top_dashed_bar(color, outline_color, bg_color, height, ratio, dash_len, gap_len,
                                                      offset.value_or(5.f));
                },
                "add_bottom_dashed_bar",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const omath::Color& outline_color,
                   const omath::Color& bg_color, const float height, const float ratio, const float dash_len,
                   const float gap_len, sol::optional<float> offset) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_bottom_dashed_bar(color, outline_color, bg_color, height, ratio, dash_len,
                                                         gap_len, offset.value_or(5.f));
                },

                "add_right_label",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const float offset,
                   const bool outlined, const std::string& text) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_right_label(color, offset, outlined, text);
                },
                "add_left_label",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const float offset,
                   const bool outlined, const std::string& text) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_left_label(color, offset, outlined, text);
                },
                "add_top_label",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const float offset,
                   const bool outlined, const std::string& text) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_top_label(color, offset, outlined, text);
                },
                "add_bottom_label",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const float offset,
                   const bool outlined, const std::string& text) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_bottom_label(color, offset, outlined, text);
                },
                "add_centered_top_label",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const float offset,
                   const bool outlined, const std::string& text) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_centered_top_label(color, offset, outlined, text);
                },
                "add_centered_bottom_label",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const float offset,
                   const bool outlined, const std::string& text) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_centered_bottom_label(color, offset, outlined, text);
                },

                "add_right_space_vertical", &omath::hud::EntityOverlay::add_right_space_vertical,
                "add_right_space_horizontal", &omath::hud::EntityOverlay::add_right_space_horizontal,
                "add_left_space_vertical", &omath::hud::EntityOverlay::add_left_space_vertical,
                "add_left_space_horizontal", &omath::hud::EntityOverlay::add_left_space_horizontal,
                "add_top_space_vertical", &omath::hud::EntityOverlay::add_top_space_vertical,
                "add_top_space_horizontal", &omath::hud::EntityOverlay::add_top_space_horizontal,
                "add_bottom_space_vertical", &omath::hud::EntityOverlay::add_bottom_space_vertical,
                "add_bottom_space_horizontal", &omath::hud::EntityOverlay::add_bottom_space_horizontal,

                "add_right_progress_ring",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const omath::Color& bg,
                   const float radius, const float ratio, sol::optional<float> thickness, sol::optional<float> offset,
                   sol::optional<int> segments) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_right_progress_ring(color, bg, radius, ratio, thickness.value_or(2.f),
                                                           offset.value_or(5.f), segments.value_or(0));
                },
                "add_left_progress_ring",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const omath::Color& bg,
                   const float radius, const float ratio, sol::optional<float> thickness, sol::optional<float> offset,
                   sol::optional<int> segments) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_left_progress_ring(color, bg, radius, ratio, thickness.value_or(2.f),
                                                          offset.value_or(5.f), segments.value_or(0));
                },
                "add_top_progress_ring",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const omath::Color& bg,
                   const float radius, const float ratio, sol::optional<float> thickness, sol::optional<float> offset,
                   sol::optional<int> segments) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_top_progress_ring(color, bg, radius, ratio, thickness.value_or(2.f),
                                                         offset.value_or(5.f), segments.value_or(0));
                },
                "add_bottom_progress_ring",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color, const omath::Color& bg,
                   const float radius, const float ratio, sol::optional<float> thickness, sol::optional<float> offset,
                   sol::optional<int> segments) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_bottom_progress_ring(color, bg, radius, ratio, thickness.value_or(2.f),
                                                            offset.value_or(5.f), segments.value_or(0));
                },

                "add_right_icon",
                [](omath::hud::EntityOverlay& overlay, const sol::object& texture_id, const float width,
                   const float height, sol::optional<omath::Color> tint,
                   sol::optional<float> offset) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_right_icon(texture_id_from_lua_object(texture_id), width, height,
                                                  tint.value_or(opaque_white()), offset.value_or(5.f));
                },
                "add_left_icon",
                [](omath::hud::EntityOverlay& overlay, const sol::object& texture_id, const float width,
                   const float height, sol::optional<omath::Color> tint,
                   sol::optional<float> offset) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_left_icon(texture_id_from_lua_object(texture_id), width, height,
                                                 tint.value_or(opaque_white()), offset.value_or(5.f));
                },
                "add_top_icon",
                [](omath::hud::EntityOverlay& overlay, const sol::object& texture_id, const float width,
                   const float height, sol::optional<omath::Color> tint,
                   sol::optional<float> offset) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_top_icon(texture_id_from_lua_object(texture_id), width, height,
                                                tint.value_or(opaque_white()), offset.value_or(5.f));
                },
                "add_bottom_icon",
                [](omath::hud::EntityOverlay& overlay, const sol::object& texture_id, const float width,
                   const float height, sol::optional<omath::Color> tint,
                   sol::optional<float> offset) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_bottom_icon(texture_id_from_lua_object(texture_id), width, height,
                                                   tint.value_or(opaque_white()), offset.value_or(5.f));
                },

                "add_snap_line", &omath::hud::EntityOverlay::add_snap_line, "add_skeleton",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color,
                   sol::optional<float> thickness) -> omath::hud::EntityOverlay&
                {
                    return overlay.add_skeleton(color, thickness.value_or(1.f));
                },
                "add_scan_marker",
                [](omath::hud::EntityOverlay& overlay, const omath::Color& color,
                   sol::optional<omath::Color> outline, sol::optional<float> outline_thickness)
                        -> omath::hud::EntityOverlay&
                {
                    return overlay.contents(omath::hud::widget::ScanMarker{
                            color, outline.value_or(transparent_black()), outline_thickness.value_or(1.f)});
                },
                "add_aim_dot",
                [](omath::hud::EntityOverlay& overlay, const omath::Vector2<float>& position,
                   const omath::Color& color, sol::optional<float> radius) -> omath::hud::EntityOverlay&
                {
                    return overlay.contents(omath::hud::widget::AimDot{position, color, radius.value_or(3.f)});
                },
                "add_projectile_aim",
                [](omath::hud::EntityOverlay& overlay, const omath::Vector2<float>& position,
                   const omath::Color& color, sol::optional<float> size, sol::optional<float> line_size,
                   sol::optional<omath::hud::widget::ProjectileAim::Figure> figure) -> omath::hud::EntityOverlay&
                {
                    return overlay.contents(omath::hud::widget::ProjectileAim{
                            position, color, size.value_or(3.f), line_size.value_or(1.f),
                            figure.value_or(omath::hud::widget::ProjectileAim::Figure::SQUARE)});
                });

        hud_table.new_enum("ProjectileAimFigure", "CIRCLE", omath::hud::widget::ProjectileAim::Figure::CIRCLE,
                           "SQUARE", omath::hud::widget::ProjectileAim::Figure::SQUARE);
    }
} // namespace omath::lua
#endif
