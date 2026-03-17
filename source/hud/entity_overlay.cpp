//
// Created by orange on 13.03.2026.
//
#include "omath/hud/entity_overlay.hpp"

namespace omath::hud
{
    EntityOverlay& EntityOverlay::add_2d_box(const Color& box_color, const Color& fill_color, const float thickness)
    {
        const auto points = m_canvas.as_array();

        m_renderer->add_polyline({points.data(), points.size()}, box_color, thickness);

        if (fill_color.value().w > 0.f)
            m_renderer->add_filled_polyline({points.data(), points.size()}, fill_color);

        return *this;
    }
    EntityOverlay& EntityOverlay::add_cornered_2d_box(const Color& box_color, const Color& fill_color,
                                                      const float corner_ratio_len, const float thickness)
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

        return *this;
    }
    EntityOverlay& EntityOverlay::add_right_bar(const Color& color, const Color& outline_color, const Color& bg_color,
                                                const float width, float ratio, const float offset)
    {
        ratio = std::clamp(ratio, 0.f, 1.f);
        const auto max_bar_height = std::abs(m_canvas.top_right_corner.y - m_canvas.bottom_right_corner.y);

        const auto bar_start = Vector2<float>{m_text_cursor_right.x + offset, m_canvas.bottom_right_corner.y};
        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>(width, -max_bar_height), bg_color);

        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>(width, -max_bar_height * ratio), color);
        m_renderer->add_rectangle(bar_start - Vector2<float>(1.f, 0.f),
                                  bar_start + Vector2<float>(width, -max_bar_height), outline_color);

        m_text_cursor_right.x += offset + width;

        return *this;
    }
    EntityOverlay& EntityOverlay::add_left_bar(const Color& color, const Color& outline_color, const Color& bg_color,
                                               const float width, float ratio, const float offset)
    {
        ratio = std::clamp(ratio, 0.f, 1.f);
        const auto max_bar_height = std::abs(m_canvas.top_left_corner.y - m_canvas.bottom_right_corner.y);

        const auto bar_start = Vector2<float>{m_text_cursor_left.x - (offset + width), m_canvas.bottom_left_corner.y};
        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>(width, -max_bar_height), bg_color);

        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>(width, -max_bar_height * ratio), color);
        m_renderer->add_rectangle(bar_start - Vector2<float>(1.f, 0.f),
                                  bar_start + Vector2<float>(width, -max_bar_height), outline_color);

        m_text_cursor_left.x -= offset + width;

        return *this;
    }
    EntityOverlay& EntityOverlay::add_right_label(const Color& color, const float offset, const bool outlined,
                                                  const std::string_view& text)
    {
        if (outlined)
            draw_outlined_text(m_text_cursor_right + Vector2<float>{offset, 0.f}, color, text);
        else
            m_renderer->add_text(m_text_cursor_right + Vector2<float>{offset, 0.f}, color, text.data());

        m_text_cursor_right.y += m_renderer->calc_text_size(text.data()).y;

        return *this;
    }
    EntityOverlay& EntityOverlay::add_top_label(const Color& color, const float offset, const bool outlined,
                                                const std::string_view text)
    {
        m_text_cursor_top.y -= m_renderer->calc_text_size(text.data()).y;

        if (outlined)
            draw_outlined_text(m_text_cursor_top + Vector2<float>{0.f, -offset}, color, text);
        else
            m_renderer->add_text(m_text_cursor_top + Vector2<float>{0.f, -offset}, color, text.data());

        return *this;
    }
    EntityOverlay& EntityOverlay::add_top_bar(const Color& color, const Color& outline_color, const Color& bg_color,
                                              const float height, float ratio, const float offset)
    {
        ratio = std::clamp(ratio, 0.f, 1.f);
        const auto max_bar_width = std::abs(m_canvas.top_left_corner.x - m_canvas.bottom_right_corner.x);

        const auto bar_start = Vector2<float>{m_canvas.top_left_corner.x, m_text_cursor_top.y - offset};
        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>(max_bar_width, -height), bg_color);

        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>(max_bar_width * ratio, -height), color);
        m_renderer->add_rectangle(bar_start, bar_start + Vector2<float>(max_bar_width, -height), outline_color);

        m_text_cursor_top.y -= offset + height;

        return *this;
    }
    EntityOverlay& EntityOverlay::add_snap_line(const Vector2<float>& start_pos, const Color& color, const float width)
    {
        const Vector2<float> line_end =
                m_canvas.bottom_left_corner
                + Vector2<float>{m_canvas.bottom_right_corner.x - m_canvas.bottom_left_corner.x, 0.f} / 2;
        m_renderer->add_line(start_pos, line_end, color, width);

        return *this;
    }
    void EntityOverlay::draw_dashed_fill(const Vector2<float>& origin, const Vector2<float>& step_dir,
                                         const Vector2<float>& perp_dir, const float full_len, const float filled_len,
                                         const Color& fill_color, const Color& split_color, const float dash_len,
                                         const float gap_len) const
    {
        if (full_len <= 0.f)
            return;

        const float step = dash_len + gap_len;
        const float n = std::floor((full_len + gap_len) / step);
        if (n < 1.f)
            return;

        const float used = n * dash_len + (n - 1.f) * gap_len;
        const float offset = (full_len - used) / 2.f;

        const auto fill_rect = [&](const Vector2<float>& a, const Vector2<float>& b, const Color& c)
        {
            m_renderer->add_filled_rectangle({std::min(a.x, b.x), std::min(a.y, b.y)},
                                             {std::max(a.x, b.x), std::max(a.y, b.y)}, c);
        };

        // Draw split lines (gaps) across the full bar first
        // Leading gap
        if (offset > 0.f)
            fill_rect(origin, origin + step_dir * offset + perp_dir, split_color);

        for (float i = 0.f; i < n; ++i)
        {
            const float dash_start = offset + i * step;
            const float dash_end = dash_start + dash_len;
            const float gap_start = dash_end;
            const float gap_end = dash_start + step;

            // Fill dash only up to filled_len
            if (dash_start < filled_len)
            {
                const auto a = origin + step_dir * dash_start;
                const auto b = a + step_dir * std::min(dash_len, filled_len - dash_start) + perp_dir;
                fill_rect(a, b, fill_color);
            }

            // Split line (gap) — always drawn across full bar
            if (i < n - 1.f && gap_start < full_len)
            {
                const auto a = origin + step_dir * gap_start;
                const auto b = origin + step_dir * std::min(gap_end, full_len) + perp_dir;
                fill_rect(a, b, split_color);
            }
        }

        // Trailing gap
        const float trail_start = offset + n * dash_len + (n - 1.f) * gap_len;
        if (trail_start < full_len)
            fill_rect(origin + step_dir * trail_start, origin + step_dir * full_len + perp_dir, split_color);
    }

    EntityOverlay& EntityOverlay::add_right_dashed_bar(const Color& color, const Color& outline_color,
                                                       const Color& bg_color, const float width, float ratio,
                                                       const float dash_len, const float gap_len, const float offset)
    {
        ratio = std::clamp(ratio, 0.f, 1.f);
        const float height = std::abs(m_canvas.top_right_corner.y - m_canvas.bottom_right_corner.y);
        const auto bar_start = Vector2<float>{m_text_cursor_right.x + offset, m_canvas.bottom_right_corner.y};

        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>{width, -height}, bg_color);
        draw_dashed_fill(bar_start, {0.f, -1.f}, {width, 0.f}, height, height * ratio, color, outline_color, dash_len,
                         gap_len);
        m_renderer->add_rectangle(bar_start - Vector2<float>{1.f, 0.f}, bar_start + Vector2<float>{width, -height},
                                  outline_color);
        m_text_cursor_right.x += offset + width;

        return *this;
    }

    EntityOverlay& EntityOverlay::add_left_dashed_bar(const Color& color, const Color& outline_color,
                                                      const Color& bg_color, const float width, float ratio,
                                                      const float dash_len, const float gap_len, const float offset)
    {
        ratio = std::clamp(ratio, 0.f, 1.f);
        const float height = std::abs(m_canvas.top_left_corner.y - m_canvas.bottom_left_corner.y);
        const auto bar_start = Vector2<float>{m_text_cursor_left.x - (offset + width), m_canvas.bottom_left_corner.y};

        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>{width, -height}, bg_color);
        draw_dashed_fill(bar_start, {0.f, -1.f}, {width, 0.f}, height, height * ratio, color, outline_color, dash_len,
                         gap_len);
        m_renderer->add_rectangle(bar_start - Vector2<float>{1.f, 0.f}, bar_start + Vector2<float>{width, -height},
                                  outline_color);
        m_text_cursor_left.x -= offset + width;

        return *this;
    }

    EntityOverlay& EntityOverlay::add_top_dashed_bar(const Color& color, const Color& outline_color,
                                                     const Color& bg_color, const float height, float ratio,
                                                     const float dash_len, const float gap_len, const float offset)
    {
        ratio = std::clamp(ratio, 0.f, 1.f);
        const float bar_w = std::abs(m_canvas.top_left_corner.x - m_canvas.top_right_corner.x);
        const auto bar_start = Vector2<float>{m_canvas.top_left_corner.x, m_text_cursor_top.y - offset};

        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>{bar_w, -height}, bg_color);
        draw_dashed_fill(bar_start, {1.f, 0.f}, {0.f, -height}, bar_w, bar_w * ratio, color, outline_color, dash_len,
                         gap_len);
        m_renderer->add_rectangle(bar_start, bar_start + Vector2<float>{bar_w, -height}, outline_color);
        m_text_cursor_top.y -= offset + height;

        return *this;
    }

    EntityOverlay& EntityOverlay::add_bottom_dashed_bar(const Color& color, const Color& outline_color,
                                                        const Color& bg_color, const float height, float ratio,
                                                        const float dash_len, const float gap_len, const float offset)
    {
        ratio = std::clamp(ratio, 0.f, 1.f);
        const float bar_w = std::abs(m_canvas.bottom_left_corner.x - m_canvas.bottom_right_corner.x);
        const auto bar_start = Vector2<float>{m_canvas.bottom_left_corner.x, m_text_cursor_bottom.y + offset};

        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>{bar_w, height}, bg_color);
        draw_dashed_fill(bar_start, {1.f, 0.f}, {0.f, height}, bar_w, bar_w * ratio, color, outline_color, dash_len,
                         gap_len);
        m_renderer->add_rectangle(bar_start, bar_start + Vector2<float>{bar_w, height}, outline_color);
        m_text_cursor_bottom.y += offset + height;

        return *this;
    }

    EntityOverlay& EntityOverlay::add_skeleton(const Color& color, const float thickness)
    {
        // Maps normalized (rx in [0,1], ry in [0,1]) to canvas screen position
        const auto joint = [&](const float rx, const float ry) -> Vector2<float>
        {
            const auto top = m_canvas.top_left_corner + (m_canvas.top_right_corner - m_canvas.top_left_corner) * rx;
            const auto bot =
                    m_canvas.bottom_left_corner + (m_canvas.bottom_right_corner - m_canvas.bottom_left_corner) * rx;
            return top + (bot - top) * ry;
        };

        using B = std::pair<std::pair<float, float>, std::pair<float, float>>;
        static constexpr std::array<B, 15> k_bones{{
                // Spine
                {{0.50f, 0.13f}, {0.50f, 0.22f}}, // head       → neck
                {{0.50f, 0.22f}, {0.50f, 0.38f}}, // neck       → chest
                {{0.50f, 0.38f}, {0.50f, 0.55f}}, // chest      → pelvis
                // Left arm
                {{0.50f, 0.22f}, {0.25f, 0.25f}}, // neck       → L shoulder
                {{0.25f, 0.25f}, {0.13f, 0.42f}}, // L shoulder → L elbow
                {{0.13f, 0.42f}, {0.08f, 0.56f}}, // L elbow    → L hand
                // Right arm
                {{0.50f, 0.22f}, {0.75f, 0.25f}}, // neck       → R shoulder
                {{0.75f, 0.25f}, {0.87f, 0.42f}}, // R shoulder → R elbow
                {{0.87f, 0.42f}, {0.92f, 0.56f}}, // R elbow    → R hand
                // Left leg
                {{0.50f, 0.55f}, {0.36f, 0.58f}}, // pelvis     → L hip
                {{0.36f, 0.58f}, {0.32f, 0.77f}}, // L hip      → L knee
                {{0.32f, 0.77f}, {0.27f, 0.97f}}, // L knee     → L foot
                // Right leg
                {{0.50f, 0.55f}, {0.64f, 0.58f}}, // pelvis     → R hip
                {{0.64f, 0.58f}, {0.68f, 0.77f}}, // R hip      → R knee
                {{0.68f, 0.77f}, {0.73f, 0.97f}}, // R knee     → R foot
        }};

        for (const auto& [a, b] : k_bones)
            m_renderer->add_line(joint(a.first, a.second), joint(b.first, b.second), color, thickness);

        return *this;
    }

    void EntityOverlay::draw_dashed_line(const Vector2<float>& from, const Vector2<float>& to, const Color& color,
                                         const float dash_len, const float gap_len, const float thickness) const
    {
        const auto total = (to - from).length();
        if (total <= 0.f)
            return;

        const auto dir = (to - from).normalized();
        const float step = dash_len + gap_len;

        const float n_dashes = std::floor((total + gap_len) / step);
        if (n_dashes < 1.f)
            return;

        const float used = n_dashes * dash_len + (n_dashes - 1.f) * gap_len;
        const float offset = (total - used) / 2.f;

        for (float i = 0.f; i < n_dashes; ++i)
        {
            const float pos = offset + i * step;
            const auto dash_start = from + dir * pos;
            const auto dash_end = from + dir * std::min(pos + dash_len, total);
            m_renderer->add_line(dash_start, dash_end, color, thickness);
        }
    }

    EntityOverlay& EntityOverlay::add_dashed_box(const Color& color, const float dash_len, const float gap_len,
                                                 const float thickness)
    {
        const float min_edge = std::min((m_canvas.top_right_corner - m_canvas.top_left_corner).length(),
                                        (m_canvas.bottom_right_corner - m_canvas.top_right_corner).length());
        const float corner_len = std::min(dash_len, min_edge / 2.f);

        const auto draw_edge = [&](const Vector2<float>& from, const Vector2<float>& to)
        {
            const auto dir = (to - from).normalized();

            m_renderer->add_line(from, from + dir * corner_len, color, thickness);
            draw_dashed_line(from + dir * corner_len, to - dir * corner_len, color, dash_len, gap_len, thickness);
            m_renderer->add_line(to - dir * corner_len, to, color, thickness);
        };

        draw_edge(m_canvas.top_left_corner, m_canvas.top_right_corner);
        draw_edge(m_canvas.top_right_corner, m_canvas.bottom_right_corner);
        draw_edge(m_canvas.bottom_right_corner, m_canvas.bottom_left_corner);
        draw_edge(m_canvas.bottom_left_corner, m_canvas.top_left_corner);

        return *this;
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
    EntityOverlay& EntityOverlay::add_bottom_bar(const Color& color, const Color& outline_color, const Color& bg_color,
                                                 const float height, float ratio, const float offset)
    {
        ratio = std::clamp(ratio, 0.f, 1.f);
        const auto max_bar_width = std::abs(m_canvas.bottom_right_corner.x - m_canvas.bottom_left_corner.x);

        const auto bar_start = Vector2<float>{m_canvas.bottom_left_corner.x, m_text_cursor_bottom.y + offset};
        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>(max_bar_width, height), bg_color);
        m_renderer->add_filled_rectangle(bar_start, bar_start + Vector2<float>(max_bar_width * ratio, height), color);
        m_renderer->add_rectangle(bar_start, bar_start + Vector2<float>(max_bar_width, height), outline_color);

        m_text_cursor_bottom.y += offset + height;

        return *this;
    }

    EntityOverlay& EntityOverlay::add_bottom_label(const Color& color, const float offset, const bool outlined,
                                                   const std::string_view text)
    {
        const auto text_size = m_renderer->calc_text_size(text);

        if (outlined)
            draw_outlined_text(m_text_cursor_bottom + Vector2<float>{0.f, offset}, color, text);
        else
            m_renderer->add_text(m_text_cursor_bottom + Vector2<float>{0.f, offset}, color, text);

        m_text_cursor_bottom.y += text_size.y;

        return *this;
    }

    EntityOverlay& EntityOverlay::add_left_label(const Color& color, const float offset, const bool outlined,
                                                 const std::string_view& text)
    {
        const auto text_size = m_renderer->calc_text_size(text);
        const auto pos = m_text_cursor_left + Vector2<float>{-(offset + text_size.x), 0.f};

        if (outlined)
            draw_outlined_text(pos, color, text);
        else
            m_renderer->add_text(pos, color, text);

        m_text_cursor_left.y += text_size.y;

        return *this;
    }

    EntityOverlay& EntityOverlay::add_centered_bottom_label(const Color& color, const float offset, const bool outlined,
                                                            const std::string_view& text)
    {
        const auto text_size = m_renderer->calc_text_size(text);
        const auto box_center_x =
                m_canvas.bottom_left_corner.x + (m_canvas.bottom_right_corner.x - m_canvas.bottom_left_corner.x) / 2.f;
        const auto pos = Vector2<float>{box_center_x - text_size.x / 2.f, m_text_cursor_bottom.y + offset};

        if (outlined)
            draw_outlined_text(pos, color, text);
        else
            m_renderer->add_text(pos, color, text);

        m_text_cursor_bottom.y += text_size.y;

        return *this;
    }

    EntityOverlay& EntityOverlay::add_centered_top_label(const Color& color, const float offset, const bool outlined,
                                                         const std::string_view& text)
    {
        const auto text_size = m_renderer->calc_text_size(text);
        const auto box_center_x =
                m_canvas.top_left_corner.x + (m_canvas.top_right_corner.x - m_canvas.top_left_corner.x) / 2.f;

        m_text_cursor_top.y -= text_size.y;
        const auto pos = Vector2<float>{box_center_x - text_size.x / 2.f, m_text_cursor_top.y - offset};

        if (outlined)
            draw_outlined_text(pos, color, text);
        else
            m_renderer->add_text(pos, color, text);

        return *this;
    }

    EntityOverlay::EntityOverlay(const Vector2<float>& top, const Vector2<float>& bottom,
                                 const std::shared_ptr<HudRendererInterface>& renderer)
        : m_canvas(top, bottom), m_text_cursor_right(m_canvas.top_right_corner),
          m_text_cursor_top(m_canvas.top_left_corner), m_text_cursor_bottom(m_canvas.bottom_left_corner),
          m_text_cursor_left(m_canvas.top_left_corner), m_renderer(renderer)
    {
    }
    // ── Spacers ─────────────────────────────────────────────────────────────────
    EntityOverlay& EntityOverlay::add_right_space_vertical(const float size)
    {
        m_text_cursor_right.y += size;
        return *this;
    }

    EntityOverlay& EntityOverlay::add_right_space_horizontal(const float size)
    {
        m_text_cursor_right.x += size;
        return *this;
    }

    EntityOverlay& EntityOverlay::add_left_space_vertical(const float size)
    {
        m_text_cursor_left.y += size;
        return *this;
    }

    EntityOverlay& EntityOverlay::add_left_space_horizontal(const float size)
    {
        m_text_cursor_left.x -= size;
        return *this;
    }

    EntityOverlay& EntityOverlay::add_top_space_vertical(const float size)
    {
        m_text_cursor_top.y -= size;
        return *this;
    }

    EntityOverlay& EntityOverlay::add_top_space_horizontal(const float size)
    {
        m_text_cursor_top.x += size;
        return *this;
    }

    EntityOverlay& EntityOverlay::add_bottom_space_vertical(const float size)
    {
        m_text_cursor_bottom.y += size;
        return *this;
    }

    EntityOverlay& EntityOverlay::add_bottom_space_horizontal(const float size)
    {
        m_text_cursor_bottom.x += size;
        return *this;
    }

    // ── Progress rings ──────────────────────────────────────────────────────────
    EntityOverlay& EntityOverlay::add_right_progress_ring(const Color& color, const Color& bg, const float radius,
                                                          const float ratio, const float thickness, const float offset,
                                                          const int segments)
    {
        const auto cx = m_text_cursor_right.x + offset + radius;
        const auto cy = m_text_cursor_right.y + radius;
        draw_progress_ring({cx, cy}, widget::ProgressRing{color, bg, radius, ratio, thickness, offset, segments});
        m_text_cursor_right.y += radius * 2.f;
        return *this;
    }

    EntityOverlay& EntityOverlay::add_left_progress_ring(const Color& color, const Color& bg, const float radius,
                                                         const float ratio, const float thickness, const float offset,
                                                         const int segments)
    {
        const auto cx = m_text_cursor_left.x - offset - radius;
        const auto cy = m_text_cursor_left.y + radius;
        draw_progress_ring({cx, cy}, widget::ProgressRing{color, bg, radius, ratio, thickness, offset, segments});
        m_text_cursor_left.y += radius * 2.f;
        return *this;
    }

    EntityOverlay& EntityOverlay::add_top_progress_ring(const Color& color, const Color& bg, const float radius,
                                                        const float ratio, const float thickness, const float offset,
                                                        const int segments)
    {
        m_text_cursor_top.y -= radius * 2.f;
        const auto cx = m_text_cursor_top.x + radius;
        const auto cy = m_text_cursor_top.y - offset + radius;
        draw_progress_ring({cx, cy}, widget::ProgressRing{color, bg, radius, ratio, thickness, offset, segments});
        return *this;
    }

    EntityOverlay& EntityOverlay::add_bottom_progress_ring(const Color& color, const Color& bg, const float radius,
                                                           const float ratio, const float thickness, const float offset,
                                                           const int segments)
    {
        const auto cx = m_text_cursor_bottom.x + radius;
        const auto cy = m_text_cursor_bottom.y + offset + radius;
        draw_progress_ring({cx, cy}, widget::ProgressRing{color, bg, radius, ratio, thickness, offset, segments});
        m_text_cursor_bottom.y += radius * 2.f;
        return *this;
    }

    // ── Icons ────────────────────────────────────────────────────────────────────
    EntityOverlay& EntityOverlay::add_right_icon(const std::any& texture_id, const float width, const float height,
                                                 const Color& tint, const float offset)
    {
        const auto pos = m_text_cursor_right + Vector2<float>{offset, 0.f};
        m_renderer->add_image(texture_id, pos, pos + Vector2<float>{width, height}, tint);
        m_text_cursor_right.y += height;
        return *this;
    }

    EntityOverlay& EntityOverlay::add_left_icon(const std::any& texture_id, const float width, const float height,
                                                const Color& tint, const float offset)
    {
        const auto pos = m_text_cursor_left + Vector2<float>{-(offset + width), 0.f};
        m_renderer->add_image(texture_id, pos, pos + Vector2<float>{width, height}, tint);
        m_text_cursor_left.y += height;
        return *this;
    }

    EntityOverlay& EntityOverlay::add_top_icon(const std::any& texture_id, const float width, const float height,
                                               const Color& tint, const float offset)
    {
        m_text_cursor_top.y -= height;
        const auto pos = m_text_cursor_top + Vector2<float>{0.f, -offset};
        m_renderer->add_image(texture_id, pos, pos + Vector2<float>{width, height}, tint);
        return *this;
    }

    EntityOverlay& EntityOverlay::add_bottom_icon(const std::any& texture_id, const float width, const float height,
                                                  const Color& tint, const float offset)
    {
        const auto pos = m_text_cursor_bottom + Vector2<float>{0.f, offset};
        m_renderer->add_image(texture_id, pos, pos + Vector2<float>{width, height}, tint);
        m_text_cursor_bottom.y += height;
        return *this;
    }

    // ── widget dispatch ───────────────────────────────────────────────────────
    void EntityOverlay::dispatch(const widget::Box& box)
    {
        add_2d_box(box.color, box.fill, box.thickness);
    }

    void EntityOverlay::dispatch(const widget::CorneredBox& cornered_box)
    {
        add_cornered_2d_box(cornered_box.color, cornered_box.fill, cornered_box.corner_ratio, cornered_box.thickness);
    }

    void EntityOverlay::dispatch(const widget::DashedBox& dashed_box)
    {
        add_dashed_box(dashed_box.color, dashed_box.dash_len, dashed_box.gap_len, dashed_box.thickness);
    }

    void EntityOverlay::dispatch(const widget::Skeleton& skeleton)
    {
        add_skeleton(skeleton.color, skeleton.thickness);
    }

    void EntityOverlay::dispatch(const widget::SnapLine& snap_line)
    {
        add_snap_line(snap_line.start, snap_line.color, snap_line.width);
    }

    void EntityOverlay::dispatch(const widget::ScanMarker& scan_marker)
    {
        const auto box_width = std::abs(m_canvas.top_right_corner.x - m_canvas.top_left_corner.x);
        const auto box_height = std::abs(m_canvas.bottom_left_corner.y - m_canvas.top_left_corner.y);

        const auto center_x = (m_canvas.top_left_corner.x + m_canvas.top_right_corner.x) / 2.f;
        const auto center_y = m_canvas.top_left_corner.y + box_height * 0.44f;

        const auto side = std::min(box_width, box_height) * 0.5f;
        const auto h = side * std::sqrt(3.f) / 2.f;

        const std::array<Vector2<float>, 3> tri = {
                Vector2<float>{center_x, center_y - h * 2.f / 3.f},
                Vector2<float>{center_x - side / 2.f, center_y + h / 3.f},
                Vector2<float>{center_x + side / 2.f, center_y + h / 3.f},
        };

        m_renderer->add_filled_polyline({tri.data(), tri.size()}, scan_marker.color);

        if (scan_marker.outline.value().w > 0.f)
            m_renderer->add_polyline({tri.data(), tri.size()}, scan_marker.outline, scan_marker.outline_thickness);
    }

    void EntityOverlay::dispatch(const widget::AimDot& aim_dot)
    {
        m_renderer->add_filled_circle(aim_dot.position, aim_dot.radius, aim_dot.color);
    }
    void EntityOverlay::dispatch(const widget::ProjectileAim& proj_widget)
    {
        const auto box_width = std::abs(m_canvas.top_right_corner.x - m_canvas.top_left_corner.x);
        const auto box_height = std::abs(m_canvas.bottom_left_corner.y - m_canvas.top_left_corner.y);

        const auto box_center = m_canvas.top_left_corner+Vector2{box_width, box_height} / 2.f;

        m_renderer->add_line(box_center, proj_widget.position, proj_widget.color, proj_widget.line_size);

        if (proj_widget.figure == widget::ProjectileAim::Figure::CIRCLE)
        {
            m_renderer->add_filled_circle(proj_widget.position, proj_widget.size, proj_widget.color);
        }
        else if (proj_widget.figure == widget::ProjectileAim::Figure::SQUARE)
        {
            const auto box_min = proj_widget.position - Vector2{proj_widget.size, proj_widget.size} / 2.f;
            const auto box_max = proj_widget.position + Vector2{proj_widget.size, proj_widget.size} / 2.f;
            m_renderer->add_filled_rectangle(box_min, box_max, proj_widget.color);
        }
    }

    void EntityOverlay::draw_progress_ring(const Vector2<float>& center, const widget::ProgressRing& ring)
    {
        constexpr auto pi = std::numbers::pi_v<float>;
        const float ratio = std::clamp(ring.ratio, 0.f, 1.f);

        m_renderer->add_circle(center, ring.radius, ring.bg, ring.thickness, ring.segments);

        if (ratio > 0.f)
        {
            const float a_min = -pi / 2.f;
            const float a_max = a_min + ratio * 2.f * pi;
            m_renderer->add_arc(center, ring.radius, a_min, a_max, ring.color, ring.thickness, ring.segments);
        }
    }

    // ── Side container dispatch ───────────────────────────────────────────────
    void EntityOverlay::dispatch(const widget::RightSide& right_side)
    {
        for (const auto& child : right_side.children)
            std::visit(
                    widget::Overloaded{
                            [](const widget::None&)
                            {
                            },
                            [this](const widget::Bar& w)
                            {
                                add_right_bar(w.color, w.outline, w.bg, w.size, w.ratio, w.offset);
                            },
                            [this](const widget::DashedBar& w)
                            {
                                add_right_dashed_bar(w.color, w.outline, w.bg, w.size, w.ratio, w.dash_len, w.gap_len,
                                                     w.offset);
                            },
                            [this](const widget::Label& w)
                            {
                                add_right_label(w.color, w.offset, w.outlined, w.text);
                            },
                            [this](const widget::Centered<widget::Label>& w)
                            {
                                add_right_label(w.child.color, w.child.offset, w.child.outlined, w.child.text);
                            },
                            [this](const widget::SpaceVertical& w)
                            {
                                add_right_space_vertical(w.size);
                            },
                            [this](const widget::SpaceHorizontal& w)
                            {
                                add_right_space_horizontal(w.size);
                            },
                            [this](const widget::ProgressRing& w)
                            {
                                add_right_progress_ring(w.color, w.bg, w.radius, w.ratio, w.thickness, w.offset,
                                                        w.segments);
                            },
                            [this](const widget::Icon& w)
                            {
                                add_right_icon(w.texture_id, w.width, w.height, w.tint, w.offset);
                            },
                    },
                    child);
    }

    void EntityOverlay::dispatch(const widget::LeftSide& left_side)
    {
        for (const auto& child : left_side.children)
            std::visit(
                    widget::Overloaded{
                            [](const widget::None&)
                            {
                            },
                            [this](const widget::Bar& w)
                            {
                                add_left_bar(w.color, w.outline, w.bg, w.size, w.ratio, w.offset);
                            },
                            [this](const widget::DashedBar& w)
                            {
                                add_left_dashed_bar(w.color, w.outline, w.bg, w.size, w.ratio, w.dash_len, w.gap_len,
                                                    w.offset);
                            },
                            [this](const widget::Label& w)
                            {
                                add_left_label(w.color, w.offset, w.outlined, w.text);
                            },
                            [this](const widget::Centered<widget::Label>& w)
                            {
                                add_left_label(w.child.color, w.child.offset, w.child.outlined, w.child.text);
                            },
                            [this](const widget::SpaceVertical& w)
                            {
                                add_left_space_vertical(w.size);
                            },
                            [this](const widget::SpaceHorizontal& w)
                            {
                                add_left_space_horizontal(w.size);
                            },
                            [this](const widget::ProgressRing& w)
                            {
                                add_left_progress_ring(w.color, w.bg, w.radius, w.ratio, w.thickness, w.offset,
                                                       w.segments);
                            },
                            [this](const widget::Icon& w)
                            {
                                add_left_icon(w.texture_id, w.width, w.height, w.tint, w.offset);
                            },
                    },
                    child);
    }

    void EntityOverlay::dispatch(const widget::TopSide& top_side)
    {
        for (const auto& child : top_side.children)
            std::visit(
                    widget::Overloaded{
                            [](const widget::None&)
                            {
                            },
                            [this](const widget::Bar& w)
                            {
                                add_top_bar(w.color, w.outline, w.bg, w.size, w.ratio, w.offset);
                            },
                            [this](const widget::DashedBar& w)
                            {
                                add_top_dashed_bar(w.color, w.outline, w.bg, w.size, w.ratio, w.dash_len, w.gap_len,
                                                   w.offset);
                            },
                            [this](const widget::Label& w)
                            {
                                add_top_label(w.color, w.offset, w.outlined, w.text);
                            },
                            [this](const widget::Centered<widget::Label>& w)
                            {
                                add_centered_top_label(w.child.color, w.child.offset, w.child.outlined, w.child.text);
                            },
                            [this](const widget::SpaceVertical& w)
                            {
                                add_top_space_vertical(w.size);
                            },
                            [this](const widget::SpaceHorizontal& w)
                            {
                                add_top_space_horizontal(w.size);
                            },
                            [this](const widget::ProgressRing& w)
                            {
                                add_top_progress_ring(w.color, w.bg, w.radius, w.ratio, w.thickness, w.offset,
                                                      w.segments);
                            },
                            [this](const widget::Icon& w)
                            {
                                add_top_icon(w.texture_id, w.width, w.height, w.tint, w.offset);
                            },
                    },
                    child);
    }
    void EntityOverlay::dispatch(const widget::BottomSide& bottom_side)
    {
        for (const auto& child : bottom_side.children)
            std::visit(
                    widget::Overloaded{
                            [](const widget::None&)
                            {
                            },
                            [this](const widget::Bar& w)
                            {
                                add_bottom_bar(w.color, w.outline, w.bg, w.size, w.ratio, w.offset);
                            },
                            [this](const widget::DashedBar& w)
                            {
                                add_bottom_dashed_bar(w.color, w.outline, w.bg, w.size, w.ratio, w.dash_len, w.gap_len,
                                                      w.offset);
                            },
                            [this](const widget::Label& w)
                            {
                                add_bottom_label(w.color, w.offset, w.outlined, w.text);
                            },
                            [this](const widget::Centered<widget::Label>& w)
                            {
                                add_centered_bottom_label(w.child.color, w.child.offset, w.child.outlined,
                                                          w.child.text);
                            },
                            [this](const widget::SpaceVertical& w)
                            {
                                add_bottom_space_vertical(w.size);
                            },
                            [this](const widget::SpaceHorizontal& w)
                            {
                                add_bottom_space_horizontal(w.size);
                            },
                            [this](const widget::ProgressRing& w)
                            {
                                add_bottom_progress_ring(w.color, w.bg, w.radius, w.ratio, w.thickness, w.offset,
                                                         w.segments);
                            },
                            [this](const widget::Icon& w)
                            {
                                add_bottom_icon(w.texture_id, w.width, w.height, w.tint, w.offset);
                            },
                    },
                    child);
    }

} // namespace omath::hud