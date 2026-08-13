//
// Created by orange on 13.08.2026.
//
#pragma once
#include "entity_overlay_widgets.hpp"
#include "hud_renderer_interface.hpp"
#include "omath/linear_algebra/vector2.hpp"
#include <optional>
#include <string_view>

namespace omath::hud
{
    /// Shared text-drawing routine used by EntityOverlay and ScreenOverlay: renders an
    /// optional glow halo, an optional 8-direction outline, then the label itself.
    void draw_label(HudRendererInterface& renderer, const Vector2<float>& position, const widget::Paint& paint,
                    widget::Outlined outlined, const std::string_view& text, const std::optional<widget::Glow>& glow);
} // namespace omath::hud
