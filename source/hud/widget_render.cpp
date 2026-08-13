//
// Created by orange on 13.08.2026.
//
#include "omath/hud/widget_render.hpp"
#include <algorithm>
#include <array>
#include <cmath>

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
} // namespace omath::hud
