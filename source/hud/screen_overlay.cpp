//
// Created by orange on 13.08.2026.
//
#include "omath/hud/screen_overlay.hpp"

namespace omath::hud
{
    ScreenOverlay::ScreenOverlay(const Vector2<float>& screen_size,
                                 const std::shared_ptr<HudRendererInterface>& renderer)
        : m_center(screen_size / 2.f), m_renderer(renderer)
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

    // ── widget dispatch ───────────────────────────────────────────────────────
    void ScreenOverlay::dispatch(const widget::Crosshair& crosshair)
    {
        add_crosshair(crosshair);
    }

    void ScreenOverlay::dispatch(const widget::FovCircle& fov_circle)
    {
        add_fov_circle(fov_circle);
    }
} // namespace omath::hud
