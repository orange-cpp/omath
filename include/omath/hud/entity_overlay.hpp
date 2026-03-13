//
// Created by orange on 13.03.2026.
//
#pragma once
#include "canvas_box.hpp"
#include "hud_renderer_interface.hpp"
#include "omath/linear_algebra/vector2.hpp"
#include "omath/utility/color.hpp"
#include <memory>
#include <string_view>
#include <span>
namespace omath::hud
{
    class EntityOverlay final
    {
    public:
        EntityOverlay(const Vector2<float>& top, const Vector2<float>& bottom,
                            const std::shared_ptr<HudRendererInterface>& renderer);

        void add_2d_box(const Color& box_color, const Color& fill_color = Color{0.f, 0.f, 0.f, 0.f},
                        float thickness = 1.f) const;

        void add_cornered_2d_box(const Color& box_color, const Color& fill_color = Color{0.f, 0.f, 0.f, 0.f},
                                 float corner_ratio_len = 0.2f, float thickness = 1.f) const;

        void add_right_bar(const Color& color, const Color& outline_color, const Color& bg_color, float width,
                           float ratio, float offset = 5.f);

        void add_left_bar(const Color& color, const Color& outline_color, const Color& bg_color, float width,
                          float ratio, float offset = 5.f) const;

        template<typename... Args>
        void add_right_label(const Color& color, const float offset, const bool outlined,
                             std::format_string<Args...> fmt, Args&&... args)
        {
            const std::string label = std::vformat(fmt.get(), std::make_format_args(args...));

            add_right_label(color, offset, outlined, std::string_view{label});
        }

        void add_right_label(const Color& color, float offset, bool outlined, const std::string_view& text);

        template<typename... Args>
        void add_top_label(const Color& color, const float offset, const bool outlined, std::format_string<Args...> fmt,
                           Args&&... args)
        {
            const std::string label = std::vformat(fmt.get(), std::make_format_args(args...));

            add_top_label(color, offset, outlined, std::string_view{label});
        }

        void add_top_label(const Color& color, float offset, bool outlined, std::string_view text);

        void add_top_bar(const Color& color, const Color& outline_color, const Color& bg_color, float height,
                         float ratio, float offset = 5.f);

        void add_snap_line(const Vector2<float>& start_pos, const Color& color, float width);

    private:
        void draw_outlined_text(const Vector2<float>& position, const Color& color,
                                       const std::string_view& text);
        CanvasBox m_canvas;
        Vector2<float> m_text_cursor_right;
        Vector2<float> m_text_cursor_top;
        std::shared_ptr<HudRendererInterface> m_renderer;
    };
} // namespace omath::hud