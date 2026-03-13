//
// Created by orange on 13.03.2026.
//
#include "omath/hud/entity_overlay.hpp"

namespace omath::hud
{
    void EntityOverlay::add_2d_box(const Color& box_color, const Color& fill_color, const float thickness) const
    {
        const auto points = m_canvas.as_array();

        m_renderer->add_polyline({points.data(), points.size()}, box_color, thickness);
        m_renderer->add_filled_polyline({points.data(), points.size()}, fill_color, thickness);
    }
    void EntityOverlay::add_cornered_2d_box(const Color& box_color, const Color& fill_color,
                                            const float corner_ratio_len, const float thickness) const
    {
        const auto corner_line_length =
                std::abs((m_canvas.top_left_corner - m_canvas.top_right_corner).x * corner_ratio_len);

        if (fill_color.value().w > 0.f)
            add_2d_box(fill_color, fill_color);
        // Left Side
        m_renderer->add_line(m_canvas.top_left_corner,
                             m_canvas.top_left_corner + Vector2<float>{corner_line_length, 0.f}, box_color, thickness);

        m_renderer->add_line(m_canvas.top_left_corner,
                             m_canvas.top_left_corner + Vector2<float>{0.f, corner_line_length}, box_color, thickness);

        m_renderer->add_line(m_canvas.bottom_left_corner,
                             m_canvas.bottom_left_corner - Vector2<float>{0.f, corner_line_length}, box_color,
                             thickness);

        m_renderer->add_line(m_canvas.bottom_left_corner,
                             m_canvas.bottom_left_corner + Vector2<float>{corner_line_length, 0.f}, box_color,
                             thickness);
        // Right Side
        m_renderer->add_line(m_canvas.top_right_corner,
                             m_canvas.top_right_corner - Vector2<float>{corner_line_length, 0.f}, box_color, thickness);

        m_renderer->add_line(m_canvas.top_right_corner,
                             m_canvas.top_right_corner + Vector2<float>{0.f, corner_line_length}, box_color, thickness);

        m_renderer->add_line(m_canvas.bottom_right_corner,
                             m_canvas.bottom_right_corner - Vector2<float>{0.f, corner_line_length}, box_color,
                             thickness);

        m_renderer->add_line(m_canvas.bottom_right_corner,
                             m_canvas.bottom_right_corner - Vector2<float>{corner_line_length, 0.f}, box_color,
                             thickness);
    }
    void EntityOverlay::add_right_bar(const Color& color, const Color& outline_color, const Color& bg_color,
                                      const float width, float ratio, const float offset)
    {
        ratio = std::clamp(ratio, 0.f, 1.f);
        const auto max_bar_height = std::abs(m_canvas.top_right_corner.y - m_canvas.bottom_right_corner.y);

        const auto bar_start = m_canvas.bottom_right_corner + Vector2<float>{offset, 0.f};
        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>(width, -max_bar_height), bg_color);

        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>(width, -max_bar_height * ratio), color);
        m_renderer->add_rectangle(bar_start - Vector2<float>(1.f, 0.f),
                                  bar_start + Vector2<float>(width, -max_bar_height), outline_color);

        m_text_cursor_right.x += offset + width;
    }
    void EntityOverlay::add_left_bar(const Color& color, const Color& outline_color, const Color& bg_color,
                                     const float width, float ratio, const float offset) const
    {
        ratio = std::clamp(ratio, 0.f, 1.f);
        const auto max_bar_height = std::abs(m_canvas.top_left_corner.y - m_canvas.bottom_right_corner.y);

        const auto bar_start = m_canvas.bottom_left_corner + Vector2<float>{-(offset + width), 0.f};
        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>(width, -max_bar_height), bg_color);

        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>(width, -max_bar_height * ratio), color);
        m_renderer->add_rectangle(bar_start - Vector2<float>(1.f, 0.f),
                                  bar_start + Vector2<float>(width, -max_bar_height), outline_color);
    }
    void EntityOverlay::add_right_label(const Color& color, const float offset, const bool outlined,
                                        const std::string_view& text)
    {
        if (outlined)
            draw_outlined_text(m_text_cursor_right + Vector2<float>{offset, 0.f}, color, text);
        else
            m_renderer->add_text(m_text_cursor_right + Vector2<float>{offset, 0.f}, color, text.data());

        m_text_cursor_right.y += m_renderer->calc_text_size(text.data()).y;
    }
    void EntityOverlay::add_top_label(const Color& color, const float offset, const bool outlined,
                                      const std::string_view text)
    {
        m_text_cursor_top.y -= m_renderer->calc_text_size(text.data()).y;

        if (outlined)
            draw_outlined_text(m_text_cursor_top + Vector2<float>{0.f, -offset}, color, text);
        else
            m_renderer->add_text(m_text_cursor_top + Vector2<float>{0.f, -offset}, color, text.data());
    }
    void EntityOverlay::add_top_bar(const Color& color, const Color& outline_color, const Color& bg_color,
                                    const float height, float ratio, const float offset)
    {
        ratio = std::clamp(ratio, 0.f, 1.f);
        const auto max_bar_width = std::abs(m_canvas.top_left_corner.x - m_canvas.bottom_right_corner.x);

        const auto bar_start = m_canvas.top_left_corner - Vector2<float>{0.f, offset};
        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>(max_bar_width, -height), bg_color);

        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>(max_bar_width * ratio, -height), color);
        m_renderer->add_rectangle(bar_start, bar_start + Vector2<float>(max_bar_width, -height), outline_color);

        m_text_cursor_top.y -= offset + height;
    }
    void EntityOverlay::add_snap_line(const Vector2<float>& start_pos, const Color& color, const float width)
    {
        const Vector2<float> line_end =
                m_canvas.bottom_left_corner
                + Vector2<float>{m_canvas.bottom_right_corner.x - m_canvas.bottom_left_corner.x, 0.f} / 2;
        m_renderer->add_line(start_pos, line_end, color, width);
    }
    void EntityOverlay::draw_outlined_text(const Vector2<float>& position, const Color& color,
                                           const std::string_view& text)
    {
        static constexpr std::array outline_offsets = {
                Vector2<float>{-1, -1}, Vector2<float>{-1, 0}, Vector2<float>{-1, 1}, Vector2<float>{0, -1},
                Vector2<float>{0, 1},   Vector2<float>{1, -1}, Vector2<float>{1, 0},  Vector2<float>{1, 1}};

        for (const auto& outline_offset : outline_offsets)
            m_renderer->add_text(position + outline_offset, Color{0.f, 0.f, 0.f, 1.f}, text.data());
        m_renderer->add_text(position, color, text.data());
    }
    EntityOverlay::EntityOverlay(const Vector2<float>& top, const Vector2<float>& bottom,
                                 const std::shared_ptr<HudRendererInterface>& renderer)
        : m_canvas(top, bottom), m_text_cursor_right(m_canvas.top_right_corner),
          m_text_cursor_top(m_canvas.top_left_corner), m_renderer(renderer)
    {
    }
} // namespace omath::hud