//
// Created by orange on 13.03.2026.
//
#pragma once
#include "omath/linear_algebra/vector2.hpp"
#include "omath/utility/color.hpp"
#include <span>

namespace omath::hud
{
    class HudRendererInterface
    {
    public:
        virtual ~HudRendererInterface() = default;
        virtual void add_line(const Vector2<float>& line_start, const Vector2<float>& line_end, const Color& color,
                              float thickness) = 0;

        virtual void add_polyline(const std::span<const Vector2<float>>& vertexes, const Color& color,
                                  float thickness) = 0;

        virtual void add_filled_polyline(const std::span<const Vector2<float>>& vertexes, const Color& color) = 0;

        virtual void add_rectangle(const Vector2<float>& min, const Vector2<float>& max, const Color& color) = 0;

        virtual void add_filled_rectangle(const Vector2<float>& min, const Vector2<float>& max, const Color& color) = 0;

        virtual void add_text(const Vector2<float>& position, const Color& color, const std::string_view& text) = 0;

        [[nodiscard]]
        virtual Vector2<float> calc_text_size(const std::string_view& text) = 0;
    };
} // namespace omath::hud
