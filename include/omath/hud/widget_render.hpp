//
// Created by orange on 13.08.2026.
//
#pragma once
#include "entity_overlay_widgets.hpp"
#include "hud_renderer_interface.hpp"
#include "omath/linear_algebra/vector2.hpp"
#include <optional>
#include <span>
#include <string_view>

namespace omath::hud
{
    /// Shared text-drawing routine used by EntityOverlay and ScreenOverlay: renders an
    /// optional glow halo, an optional 8-direction outline, then the label itself.
    void draw_label(HudRendererInterface& renderer, const Vector2<float>& position, const widget::Paint& paint,
                    widget::Outlined outlined, const std::string_view& text, const std::optional<widget::Glow>& glow);

    /// Draws a soft multi-layer glow halo tracing a closed polyline.
    void draw_glow_polyline(HudRendererInterface& renderer, std::span<const Vector2<float>> points,
                            const widget::Glow& glow, float thickness);

    /// Draws a glow halo around the four edges of an axis-aligned rectangle. No-op when
    /// `glow` is unset or has a non-positive radius.
    void draw_glow_rectangle(HudRendererInterface& renderer, const Vector2<float>& min, const Vector2<float>& max,
                             const std::optional<widget::Glow>& glow);

    /// Fills a rectangle with a solid color or a corner gradient, depending on which
    /// alternative `paint` currently holds.
    void draw_filled_rectangle(HudRendererInterface& renderer, const Vector2<float>& min, const Vector2<float>& max,
                               const widget::Paint& paint);

    /// Resolves a bar's BarPaint into a drawable Paint. Color/Gradient pass through
    /// unchanged; BarGradient blends toward `full_color` by `ratio` and builds a two-stop
    /// Gradient oriented along the bar's fill axis (`vertical` selects top-to-bottom vs
    /// left-to-right).
    [[nodiscard]]
    widget::Paint resolve_bar_paint(const widget::BarPaint& paint, float ratio, bool vertical);

    /// Draws a soft ambient glow bloom expanding outward from an axis-aligned rectangle's
    /// edges: layered rounded-rect strokes with smoothstep falloff, clipped per edge so the
    /// glow never bleeds into the rectangle's interior. Same algorithm as EntityOverlay's
    /// CanvasGlow, generalized to any rectangle. No-op when radius is non-positive.
    void draw_edge_glow(HudRendererInterface& renderer, const Vector2<float>& min, const Vector2<float>& max,
                        const widget::CanvasGlow& canvas_glow);

    /// Punches a dash-and-gap pattern into a bar: draws `gap_color` rectangles at each gap
    /// interval from `origin` along `step_dir` for the full `full_len`, `perp_dir` giving
    /// the bar's width. Dashes are centered within `full_len` with any remainder split into
    /// a leading/trailing gap. Meant to be called after the bar's background and fill have
    /// already been drawn as continuous rectangles (so gaps punch through both, and the fill
    /// can be any Paint — solid, gradient, or BarGradient — rather than a flat per-dash
    /// color).
    void draw_dash_gaps(HudRendererInterface& renderer, const Vector2<float>& origin, const Vector2<float>& step_dir,
                        const Vector2<float>& perp_dir, float full_len, const Color& gap_color, float dash_len,
                        float gap_len);
} // namespace omath::hud
