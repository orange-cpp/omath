//
// Created by orange on 13.08.2026.
//
#pragma once
#include "entity_overlay_widgets.hpp"
#include "hud_renderer_interface.hpp"
#include "omath/linear_algebra/vector2.hpp"
#include "omath/utility/color.hpp"
#include <memory>
#include <optional>

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

        Vector2<float> m_center;
        std::shared_ptr<HudRendererInterface> m_renderer;
    };
} // namespace omath::hud
