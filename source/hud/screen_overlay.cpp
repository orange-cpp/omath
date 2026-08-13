//
// Created by orange on 13.08.2026.
//
#include "omath/hud/screen_overlay.hpp"
#include "omath/hud/widget_render.hpp"
#include <algorithm>
#include <array>
#include <cmath>

namespace omath::hud
{
    ScreenOverlay::ScreenOverlay(const Vector2<float>& screen_size,
                                 const std::shared_ptr<HudRendererInterface>& renderer)
        : m_screen_size(screen_size), m_center(screen_size / 2.f), m_renderer(renderer)
    {
    }

    ScreenOverlay& ScreenOverlay::add_crosshair(const widget::Crosshair& crosshair)
    {
        // Arms are filled rectangles rather than lines: line rendering is nudged by half a
        // pixel in ImGui-like backends, which would shift them off the center dot.
        const auto draw_arm = [&](const Vector2<float>& min, const Vector2<float>& max)
        {
            if (crosshair.outline.value().w > 0.f)
                m_renderer->add_filled_rectangle(min - Vector2<float>{1.f, 1.f}, max + Vector2<float>{1.f, 1.f},
                                                 crosshair.outline);

            m_renderer->add_filled_rectangle(min, max, crosshair.color);
        };

        const auto inner = crosshair.gap;
        const auto outer = crosshair.gap + crosshair.length;
        const auto half_thickness = crosshair.thickness / 2.f;

        draw_arm({m_center.x - half_thickness, m_center.y - outer}, {m_center.x + half_thickness, m_center.y - inner});
        draw_arm({m_center.x - half_thickness, m_center.y + inner}, {m_center.x + half_thickness, m_center.y + outer});
        draw_arm({m_center.x - outer, m_center.y - half_thickness}, {m_center.x - inner, m_center.y + half_thickness});
        draw_arm({m_center.x + inner, m_center.y - half_thickness}, {m_center.x + outer, m_center.y + half_thickness});

        if (crosshair.dot_radius > 0.f)
        {
            if (crosshair.outline.value().w > 0.f)
                m_renderer->add_filled_circle(m_center, crosshair.dot_radius + 1.f, crosshair.outline);

            m_renderer->add_filled_circle(m_center, crosshair.dot_radius, crosshair.color);
        }

        return *this;
    }

    ScreenOverlay& ScreenOverlay::add_fov_circle(const widget::FovCircle& fov_circle)
    {
        if (fov_circle.fill.value().w > 0.f)
            m_renderer->add_filled_circle(m_center, fov_circle.radius, fov_circle.fill, fov_circle.segments);

        m_renderer->add_circle(m_center, fov_circle.radius, fov_circle.color, fov_circle.thickness,
                               fov_circle.segments);

        return *this;
    }

    ScreenOverlay& ScreenOverlay::add_threat_arrow(const widget::ThreatArrow& threat_arrow)
    {
        const Vector2<float> dir{std::cos(threat_arrow.angle), std::sin(threat_arrow.angle)};
        const Vector2<float> perp{-dir.y, dir.x};

        const auto tip = m_center + dir * threat_arrow.radius;
        const auto base = m_center + dir * (threat_arrow.radius - threat_arrow.size);
        const auto wing = perp * (threat_arrow.size * 0.6f);

        const std::array<Vector2<float>, 3> tri = {tip, base + wing, base - wing};

        if (threat_arrow.outline.value().w > 0.f)
            m_renderer->add_polyline({tri.data(), tri.size()}, threat_arrow.outline, 2.f);

        m_renderer->add_filled_polyline({tri.data(), tri.size()}, threat_arrow.color);

        return *this;
    }

    ScreenOverlay& ScreenOverlay::add_hit_marker(const widget::HitMarker& hit_marker)
    {
        const auto fade = std::clamp(hit_marker.alpha, 0.f, 1.f);
        const auto value = hit_marker.color.value();
        const Color color{value.x, value.y, value.z, value.w * fade};
        const auto outline_value = hit_marker.outline.value();
        const Color outline{outline_value.x, outline_value.y, outline_value.z, outline_value.w * fade};

        static constexpr float k_diag = 0.70710678f;
        static constexpr std::array<Vector2<float>, 4> directions = {
                Vector2<float>{k_diag, k_diag}, Vector2<float>{k_diag, -k_diag}, Vector2<float>{-k_diag, k_diag},
                Vector2<float>{-k_diag, -k_diag}};

        for (const auto& dir : directions)
        {
            const auto from = m_center + dir * hit_marker.gap;
            const auto to = m_center + dir * (hit_marker.gap + hit_marker.size);

            if (outline_value.w > 0.f)
                m_renderer->add_line(from, to, outline, hit_marker.thickness + 2.f);

            m_renderer->add_line(from, to, color, hit_marker.thickness);
        }

        return *this;
    }

    ScreenOverlay& ScreenOverlay::add_corner(const widget::Corner& corner)
    {
        using widget::Anchor;
        const bool is_top = corner.anchor == Anchor::TOP_LEFT || corner.anchor == Anchor::TOP_CENTER
                            || corner.anchor == Anchor::TOP_RIGHT;
        const bool is_left = corner.anchor == Anchor::TOP_LEFT || corner.anchor == Anchor::BOTTOM_LEFT;
        const bool is_right = corner.anchor == Anchor::TOP_RIGHT || corner.anchor == Anchor::BOTTOM_RIGHT;

        float cursor_y = is_top ? corner.padding : m_screen_size.y - corner.padding;

        const auto resolve_x = [&](const float width) -> float
        {
            if (is_left)
                return corner.padding;
            if (is_right)
                return m_screen_size.x - corner.padding - width;
            return (m_screen_size.x - width) / 2.f;
        };

        for (const auto& child : corner.children)
            std::visit(
                    widget::Overloaded{
                            [](const widget::None&)
                            {
                            },
                            [&](const widget::Label& w)
                            {
                                const auto size = m_renderer->calc_text_size(w.text);
                                if (is_top)
                                {
                                    draw_label(*m_renderer, {resolve_x(size.x), cursor_y + w.offset}, w.color,
                                               w.outlined, w.text, w.glow);
                                    cursor_y += size.y;
                                }
                                else
                                {
                                    cursor_y -= size.y;
                                    draw_label(*m_renderer, {resolve_x(size.x), cursor_y - w.offset}, w.color,
                                               w.outlined, w.text, w.glow);
                                }
                            },
                            [&](const widget::SpaceVertical& w)
                            {
                                cursor_y += is_top ? w.size : -w.size;
                            },
                    },
                    child);

        return *this;
    }

    // ── widget dispatch ───────────────────────────────────────────────────────
    void ScreenOverlay::dispatch(const widget::Crosshair& crosshair)
    {
        add_crosshair(crosshair);
    }

    void ScreenOverlay::dispatch(const widget::FovCircle& fov_circle)
    {
        add_fov_circle(fov_circle);
    }

    void ScreenOverlay::dispatch(const widget::ThreatArrow& threat_arrow)
    {
        add_threat_arrow(threat_arrow);
    }

    void ScreenOverlay::dispatch(const widget::HitMarker& hit_marker)
    {
        add_hit_marker(hit_marker);
    }

    void ScreenOverlay::dispatch(const widget::Corner& corner)
    {
        add_corner(corner);
    }
} // namespace omath::hud
