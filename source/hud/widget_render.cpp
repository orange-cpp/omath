//
// Created by orange on 13.08.2026.
//
#include "omath/hud/widget_render.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numbers>

namespace omath::hud
{
    void draw_label(HudRendererInterface& renderer, const Vector2<float>& position, const widget::Paint& paint,
                    const widget::Outlined outlined, const std::string_view& text,
                    const std::optional<widget::Glow>& glow)
    {
        if (glow)
        {
            const int radius = static_cast<int>(std::ceil(std::max(glow->radius, 0.f)));
            for (int layer = radius; layer > 0; --layer)
            {
                const float alpha = glow->color.value().w * std::max(glow->intensity, 0.f)
                                    * (1.f - static_cast<float>(layer - 1) / static_cast<float>(radius + 1));
                const auto value = glow->color.value();
                const Color color{value.x, value.y, value.z, alpha / static_cast<float>(radius)};
                for (int x = -layer; x <= layer; ++x)
                {
                    renderer.add_text(position + Vector2<float>{static_cast<float>(x), static_cast<float>(-layer)},
                                      color, text);
                    renderer.add_text(position + Vector2<float>{static_cast<float>(x), static_cast<float>(layer)},
                                      color, text);
                }
                for (int y = -layer + 1; y < layer; ++y)
                {
                    renderer.add_text(position + Vector2<float>{static_cast<float>(-layer), static_cast<float>(y)},
                                      color, text);
                    renderer.add_text(position + Vector2<float>{static_cast<float>(layer), static_cast<float>(y)},
                                      color, text);
                }
            }
        }

        if (outlined == widget::Outlined::On)
        {
            static constexpr std::array outline_offsets = {
                    Vector2<float>{-1, -1}, Vector2<float>{-1, 0}, Vector2<float>{-1, 1}, Vector2<float>{0, -1},
                    Vector2<float>{0, 1},   Vector2<float>{1, -1}, Vector2<float>{1, 0},  Vector2<float>{1, 1}};

            for (const auto& outline_offset : outline_offsets)
                renderer.add_text(position + outline_offset, Color{0.f, 0.f, 0.f, 1.f}, text);
        }

        std::visit(
                widget::Overloaded{
                        [&](const Color& color)
                        {
                            renderer.add_text(position, color, text);
                        },
                        [&](const Gradient& gradient)
                        {
                            renderer.add_gradient_text(position, gradient, text);
                        },
                },
                paint);
    }

    void draw_glow_polyline(HudRendererInterface& renderer, const std::span<const Vector2<float>> points,
                            const widget::Glow& glow, const float thickness)
    {
        const int radius = static_cast<int>(std::ceil(std::max(glow.radius, 0.f)));
        const auto value = glow.color.value();
        for (int layer = radius; layer > 0; --layer)
        {
            const float falloff = 1.f - static_cast<float>(layer - 1) / static_cast<float>(radius + 1);
            const Color color{value.x, value.y, value.z,
                              value.w * std::max(glow.intensity, 0.f) * falloff / static_cast<float>(radius)};
            renderer.add_polyline(points, color, thickness + static_cast<float>(layer) * 2.f);
        }
    }

    void draw_glow_rectangle(HudRendererInterface& renderer, const Vector2<float>& min, const Vector2<float>& max,
                             const std::optional<widget::Glow>& glow)
    {
        if (!glow || glow->radius <= 0.f)
            return;

        const std::array points = {
                Vector2<float>{std::min(min.x, max.x), std::min(min.y, max.y)},
                Vector2<float>{std::max(min.x, max.x), std::min(min.y, max.y)},
                Vector2<float>{std::max(min.x, max.x), std::max(min.y, max.y)},
                Vector2<float>{std::min(min.x, max.x), std::max(min.y, max.y)},
        };
        draw_glow_polyline(renderer, points, *glow, 1.f);
    }

    void draw_filled_rectangle(HudRendererInterface& renderer, const Vector2<float>& min, const Vector2<float>& max,
                               const widget::Paint& paint)
    {
        const Vector2<float> top_left{std::min(min.x, max.x), std::min(min.y, max.y)};
        const Vector2<float> bottom_right{std::max(min.x, max.x), std::max(min.y, max.y)};
        std::visit(
                widget::Overloaded{
                        [&](const Color& color)
                        {
                            renderer.add_filled_rectangle(top_left, bottom_right, color);
                        },
                        [&](const Gradient& gradient)
                        {
                            renderer.add_gradient_rectangle(top_left, bottom_right, gradient);
                        },
                },
                paint);
    }

    widget::Paint resolve_bar_paint(const widget::BarPaint& paint, const float ratio, const bool vertical)
    {
        return std::visit(
                widget::Overloaded{
                        [](const Color& color) -> widget::Paint
                        {
                            return color;
                        },
                        [](const Gradient& gradient) -> widget::Paint
                        {
                            return gradient;
                        },
                        [&](const widget::BarGradient& gradient) -> widget::Paint
                        {
                            const float value = std::clamp(ratio, 0.f, 1.f);
                            const auto current = Color{gradient.empty_color.value() * (1.f - value)
                                                       + gradient.full_color.value() * value};
                            if (vertical)
                                return Gradient{current, current, gradient.empty_color, gradient.empty_color};
                            return Gradient{gradient.empty_color, current, current, gradient.empty_color};
                        },
                },
                paint);
    }

    void draw_edge_glow(HudRendererInterface& renderer, const Vector2<float>& min, const Vector2<float>& max,
                        const widget::CanvasGlow& canvas_glow)
    {
        if (canvas_glow.glow.radius <= 0.f)
            return;

        const int layers = std::max(canvas_glow.layers, 2);
        const auto value = canvas_glow.glow.color.value();
        const float intensity = std::max(canvas_glow.glow.intensity, 0.f);

        constexpr int corner_segments = 12;
        const float max_rounding = std::min(std::abs(max.x - min.x), std::abs(max.y - min.y)) * 0.5f;
        const float rounding = std::clamp(canvas_glow.rounding, 0.f, max_rounding);
        const float extent = canvas_glow.glow.radius;
        const float stroke = std::max(extent / static_cast<float>(layers - 1) * 2.f, 1.f);
        for (int layer = 0; layer < layers; ++layer)
        {
            const float progress = static_cast<float>(layer) / static_cast<float>(layers - 1);
            const float expansion = extent * (1.f - progress);
            const float falloff = progress * progress * (3.f - 2.f * progress);
            const Color color{value.x, value.y, value.z, value.w * intensity * falloff * 0.35f};
            const auto expanded_min = min - Vector2<float>{expansion, expansion};
            const auto expanded_max = max + Vector2<float>{expansion, expansion};
            const float expanded_rounding = rounding + expansion;
            const std::array corner_centers = {
                    expanded_min + Vector2<float>{expanded_rounding, expanded_rounding},
                    Vector2<float>{expanded_max.x - expanded_rounding, expanded_min.y + expanded_rounding},
                    expanded_max - Vector2<float>{expanded_rounding, expanded_rounding},
                    Vector2<float>{expanded_min.x + expanded_rounding, expanded_max.y - expanded_rounding},
            };
            std::array<Vector2<float>, corner_segments * 4> points;
            for (int corner = 0; corner < 4; ++corner)
            {
                const float start_angle =
                        std::numbers::pi_v<float> + static_cast<float>(corner) * std::numbers::pi_v<float> * 0.5f;
                for (int segment = 0; segment < corner_segments; ++segment)
                {
                    const float arc_progress = static_cast<float>(segment) / static_cast<float>(corner_segments - 1);
                    const float angle = start_angle + arc_progress * std::numbers::pi_v<float> * 0.5f;
                    points[corner * corner_segments + segment] =
                            corner_centers[corner]
                            + Vector2<float>{std::cos(angle) * expanded_rounding, std::sin(angle) * expanded_rounding};
                }
            }
            constexpr float clip_extent = std::numeric_limits<float>::max() * 0.25f;
            renderer.add_polyline_clipped(points, {-clip_extent, -clip_extent}, {clip_extent, min.y}, color, stroke);
            renderer.add_polyline_clipped(points, {-clip_extent, max.y}, {clip_extent, clip_extent}, color, stroke);
            renderer.add_polyline_clipped(points, {-clip_extent, min.y}, {min.x, max.y}, color, stroke);
            renderer.add_polyline_clipped(points, {max.x, min.y}, {clip_extent, max.y}, color, stroke);
        }
    }

    void draw_dash_gaps(HudRendererInterface& renderer, const Vector2<float>& origin, const Vector2<float>& step_dir,
                        const Vector2<float>& perp_dir, const float full_len, const Color& gap_color,
                        const float dash_len, const float gap_len)
    {
        if (full_len <= 0.f)
            return;

        const float step = dash_len + gap_len;
        const float n = std::floor((full_len + gap_len) / step);
        if (n < 1.f)
            return;

        const float used = n * dash_len + (n - 1.f) * gap_len;
        const float offset = (full_len - used) / 2.f;

        const auto fill_rect = [&](const Vector2<float>& a, const Vector2<float>& b)
        {
            renderer.add_filled_rectangle({std::min(a.x, b.x), std::min(a.y, b.y)},
                                          {std::max(a.x, b.x), std::max(a.y, b.y)}, gap_color);
        };

        // Leading gap
        if (offset > 0.f)
            fill_rect(origin, origin + step_dir * offset + perp_dir);

        for (float i = 0.f; i < n; ++i)
        {
            const float dash_start = offset + i * step;
            const float gap_start = dash_start + dash_len;
            const float gap_end = dash_start + step;

            // Gap — always drawn across the full bar, regardless of fill amount
            if (i < n - 1.f && gap_start < full_len)
            {
                const auto a = origin + step_dir * gap_start;
                const auto b = origin + step_dir * std::min(gap_end, full_len) + perp_dir;
                fill_rect(a, b);
            }
        }

        // Trailing gap
        const float trail_start = offset + n * dash_len + (n - 1.f) * gap_len;
        if (trail_start < full_len)
            fill_rect(origin + step_dir * trail_start, origin + step_dir * full_len + perp_dir);
    }
} // namespace omath::hud
