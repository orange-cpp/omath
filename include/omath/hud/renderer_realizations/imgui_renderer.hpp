//
// Created by orange on 13.03.2026.
//
#pragma once
#include <omath/hud/hud_renderer_interface.hpp>
namespace omath::hud
{
    class ImguiHudRenderer final : public HudRendererInterface
    {
    public:
        ~ImguiHudRenderer() override;
        void add_line(const Vector2<float>& line_start, const Vector2<float>& line_end, const Color& color,
                      float thickness) override;
        void add_polyline(const std::span<const Vector2<float>>& vertexes, float thickness) override;
        void add_filled_polyline(const std::span<const Vector2<float>>& vertexes, float thickness) override;
        void add_rectangle(const Vector2<float>& min, const Vector2<float>& max, const Color& color) override;
        void add_filled_rectangle(const Vector2<float>& min, const Vector2<float>& max, const Color& color) override;
        void add_text(const Vector2<float>& position, const Color& color, const std::string_view& text) override;
    };
}