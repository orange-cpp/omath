//
// Created by orange on 13.08.2026.
//
#pragma once
#include "entity_overlay_widgets.hpp"
#include "hud_renderer_interface.hpp"
#include "omath/linear_algebra/vector2.hpp"
#include "omath/utility/color.hpp"
#include <initializer_list>
#include <memory>
#include <optional>
#include <variant>

namespace omath::hud::widget
{
    /// Crosshair drawn in the center of the screen. `gap` is the empty space between the
    /// center and each line, `length` is the length of a single line.
    struct Crosshair
    {
        Color color;
        float gap = 4.f;
        float length = 8.f;
        float thickness = 1.f;
        Color outline{0.f, 0.f, 0.f, 0.f};
        float dot_radius = 0.f;
    };

    /// Field-of-view boundary circle drawn around the center of the screen.
    struct FovCircle
    {
        Color color;
        float radius;
        Color fill{0.f, 0.f, 0.f, 0.f};
        float thickness = 1.f;
        int segments = 0;
    };

    /// Arrow pointing from the screen center toward an off-screen threat. `angle` uses the
    /// same convention as HudRendererInterface::add_arc: radians, 0 = right (+X).
    struct ThreatArrow
    {
        float angle;
        Color color;
        float radius = 100.f;
        float size = 12.f;
        Color outline{0.f, 0.f, 0.f, 0.f};
    };

    /// Transient hit-confirm marker: four diagonal ticks around the screen center.
    /// `alpha` is a caller-driven fade multiplier (e.g. time-since-hit / fade_duration) —
    /// the widget owns no clock of its own.
    struct HitMarker
    {
        Color color;
        float size = 8.f;
        float gap = 3.f;
        float thickness = 2.f;
        Color outline{0.f, 0.f, 0.f, 0.f};
        float alpha = 1.f;
    };

    /// Vertical bar flanking the screen center — pair a LEFT and a RIGHT instance to build a
    /// `<bar> ... <bar>` layout around a Crosshair (or any other center widget; CenterBar is
    /// not coupled to Crosshair). Supports the same paint vocabulary as EntityOverlay's
    /// bars: solid/gradient/BarGradient fill, outline, background, and glow. `length` is the
    /// bar's vertical extent, centered on the screen's vertical middle; `thickness` is its
    /// horizontal extent. Fill grows bottom-to-top as `ratio` increases, matching
    /// EntityOverlay's RightSide/LeftSide bars. `glow` wraps just the filled portion (so it
    /// grows with the bar) and uses the same layered, rounded, smoothstep-falloff bloom as
    /// EntityOverlay's CanvasGlow rather than a plain expanding-stroke glow.
    struct CenterBar
    {
        enum class Side
        {
            LEFT,
            RIGHT,
        };

        Side side;
        BarPaint color;
        Color outline{0.f, 0.f, 0.f, 0.f};
        Color bg{0.f, 0.f, 0.f, 0.f};
        float length;
        float thickness = 6.f;
        float ratio;
        float offset = 20.f;
        std::optional<CanvasGlow> glow = std::nullopt;
    };

    /// Dashed counterpart to CenterBar, matching EntityOverlay's Bar/DashedBar split. Same
    /// positioning, bottom-to-top fill direction, and BarPaint (solid/gradient/BarGradient)
    /// fill support as CenterBar; unlike CenterBar it has no glow. `outline` is also the
    /// color drawn in the gaps between dashes.
    struct DashedCenterBar
    {
        CenterBar::Side side;
        BarPaint color;
        Color outline{0.f, 0.f, 0.f, 0.f};
        Color bg{0.f, 0.f, 0.f, 0.f};
        float length;
        float thickness = 6.f;
        float ratio;
        float dash_len = 8.f;
        float gap_len = 5.f;
        float offset = 20.f;
    };

    // ── Corner anchors ──────────────────────────────────────────────────────
    enum class Anchor
    {
        TOP_LEFT,
        TOP_CENTER,
        TOP_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_CENTER,
        BOTTOM_RIGHT,
    };

    using CornerWidget = std::variant<None, Label, SpaceVertical>;

    /// Vertical stack of widgets anchored to a screen corner or edge. Items are laid out
    /// in declaration order, growing away from the anchored edge (downward from TOP_*,
    /// upward from BOTTOM_*). Horizontal alignment follows the anchor: LEFT/RIGHT align
    /// to that edge, CENTER centers each item's width. `Label::offset` adds extra distance
    /// from the anchored edge for that one item, same role it plays in EntityOverlay's
    /// side containers.
    struct Corner
    {
        Anchor anchor;
        std::initializer_list<CornerWidget> children;
        float padding = 10.f;

        Corner(const Anchor a, const std::initializer_list<CornerWidget> c, const float p = 10.f)
            : anchor(a), children(c), padding(p)
        {
        }
    };
} // namespace omath::hud::widget

namespace omath::hud
{
    /// Overlay for widgets positioned relative to the screen instead of an entity.
    class ScreenOverlay final
    {
    public:
        ScreenOverlay(const Vector2<float>& screen_size, const std::shared_ptr<HudRendererInterface>& renderer);

        ScreenOverlay& add_crosshair(const widget::Crosshair& crosshair);
        ScreenOverlay& add_fov_circle(const widget::FovCircle& fov_circle);
        ScreenOverlay& add_threat_arrow(const widget::ThreatArrow& threat_arrow);
        ScreenOverlay& add_hit_marker(const widget::HitMarker& hit_marker);
        ScreenOverlay& add_center_bar(const widget::CenterBar& center_bar);
        ScreenOverlay& add_dashed_center_bar(const widget::DashedCenterBar& dashed_center_bar);
        ScreenOverlay& add_corner(const widget::Corner& corner);

        // ── Declarative interface ─────────────────────────────────────────
        /// Pass any combination of screen widget:: descriptor structs (and std::optional<W>
        /// from when()) to render them all in declaration order.
        template<typename... Widgets>
        ScreenOverlay& contents(Widgets&&... widgets)
        {
            (dispatch(std::forward<Widgets>(widgets)), ...);
            return *this;
        }

    private:
        // optional<W> dispatch — enables when() conditional widgets
        template<typename W>
        void dispatch(const std::optional<W>& w)
        {
            if (w)
                dispatch(*w);
        }

        void dispatch(const widget::Crosshair& crosshair);
        void dispatch(const widget::FovCircle& fov_circle);
        void dispatch(const widget::ThreatArrow& threat_arrow);
        void dispatch(const widget::HitMarker& hit_marker);
        void dispatch(const widget::CenterBar& center_bar);
        void dispatch(const widget::DashedCenterBar& dashed_center_bar);
        void dispatch(const widget::Corner& corner);

        Vector2<float> m_screen_size;
        Vector2<float> m_center;
        std::shared_ptr<HudRendererInterface> m_renderer;
    };
} // namespace omath::hud
