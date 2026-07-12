//
// Created by orange on 13.03.2026.
//
#include "omath/hud/renderer_realizations/imgui_renderer.hpp"

#ifdef OMATH_IMGUI_INTEGRATION
#include <cmath>
#include <imgui.h>
#include <imgui_internal.h>

namespace omath::hud
{
    ImguiHudRenderer::~ImguiHudRenderer() = default;

    void ImguiHudRenderer::add_line(const Vector2<float>& line_start, const Vector2<float>& line_end,
                                    const Color& color, const float thickness)
    {
        ImGui::GetBackgroundDrawList()->AddLine(line_start.to_im_vec2(), line_end.to_im_vec2(), color.to_im_color(),
                                                thickness);
    }

    void ImguiHudRenderer::add_polyline(const std::span<const Vector2<float>>& vertexes, const Color& color,
                                        const float thickness)
    {
        ImGui::GetBackgroundDrawList()->AddPolyline(reinterpret_cast<const ImVec2*>(vertexes.data()),
                                                    static_cast<int>(vertexes.size()), color.to_im_color(),
                                                    ImDrawFlags_Closed, thickness);
    }

    void ImguiHudRenderer::add_polyline_clipped(const std::span<const Vector2<float>>& vertexes,
                                                const Vector2<float>& clip_min, const Vector2<float>& clip_max,
                                                const Color& color, const float thickness)
    {
        auto* draw_list = ImGui::GetBackgroundDrawList();
        draw_list->PushClipRect(clip_min.to_im_vec2(), clip_max.to_im_vec2(), true);
        draw_list->AddPolyline(reinterpret_cast<const ImVec2*>(vertexes.data()), static_cast<int>(vertexes.size()),
                               color.to_im_color(), ImDrawFlags_Closed, thickness);
        draw_list->PopClipRect();
    }

    void ImguiHudRenderer::add_filled_polyline(const std::span<const Vector2<float>>& vertexes, const Color& color)
    {
        ImGui::GetBackgroundDrawList()->AddConvexPolyFilled(reinterpret_cast<const ImVec2*>(vertexes.data()),
                                                            static_cast<int>(vertexes.size()), color.to_im_color());
    }

    void ImguiHudRenderer::add_rectangle(const Vector2<float>& min, const Vector2<float>& max, const Color& color)
    {
        ImGui::GetBackgroundDrawList()->AddRect(min.to_im_vec2(), max.to_im_vec2(), color.to_im_color());
    }

    void ImguiHudRenderer::add_filled_rectangle(const Vector2<float>& min, const Vector2<float>& max,
                                                const Color& color)
    {
        ImGui::GetBackgroundDrawList()->AddRectFilled(min.to_im_vec2(), max.to_im_vec2(), color.to_im_color());
    }

    void ImguiHudRenderer::add_gradient_rectangle(const Vector2<float>& min, const Vector2<float>& max,
                                                  const Gradient& gradient)
    {
        ImGui::GetBackgroundDrawList()->AddRectFilledMultiColor(
                min.to_im_vec2(), max.to_im_vec2(), gradient.top_left.to_im_color(), gradient.top_right.to_im_color(),
                gradient.bottom_right.to_im_color(), gradient.bottom_left.to_im_color());
    }

    void ImguiHudRenderer::add_circle(const Vector2<float>& center, const float radius, const Color& color,
                                      const float thickness, const int segments)
    {
        ImGui::GetBackgroundDrawList()->AddCircle(center.to_im_vec2(), radius, color.to_im_color(), segments,
                                                  thickness);
    }

    void ImguiHudRenderer::add_filled_circle(const Vector2<float>& center, const float radius, const Color& color,
                                             const int segments)
    {
        ImGui::GetBackgroundDrawList()->AddCircleFilled(center.to_im_vec2(), radius, color.to_im_color(), segments);
    }

    void ImguiHudRenderer::add_arc(const Vector2<float>& center, const float radius, const float a_min,
                                   const float a_max, const Color& color, const float thickness, const int segments)
    {
        ImGui::GetBackgroundDrawList()->PathArcTo(center.to_im_vec2(), radius, a_min, a_max, segments);
        ImGui::GetBackgroundDrawList()->PathStroke(color.to_im_color(), ImDrawFlags_None, thickness);
    }

    void ImguiHudRenderer::add_image(const std::any& texture_id, const Vector2<float>& min, const Vector2<float>& max,
                                     const Color& tint)
    {
        ImGui::GetBackgroundDrawList()->AddImage(std::any_cast<ImTextureID>(texture_id), min.to_im_vec2(),
                                                 max.to_im_vec2(), {0, 0}, {1, 1}, tint.to_im_color());
    }

    void ImguiHudRenderer::add_text(const Vector2<float>& position, const Color& color, const std::string_view& text)
    {
        ImGui::GetBackgroundDrawList()->AddText(position.to_im_vec2(), color.to_im_color(), text.data(),
                                                text.data() + text.size());
    }

    void ImguiHudRenderer::add_gradient_text(const Vector2<float>& position, const Gradient& gradient,
                                             const std::string_view& text)
    {
        auto* draw_list = ImGui::GetBackgroundDrawList();
        if (gradient.animated)
        {
            const float animation_offset = static_cast<float>(ImGui::GetTime()) * gradient.animation_speed;
            const float direction = gradient.direction == GradientDirection::RightToLeft ? 1.f : -1.f;
            float cursor_x = position.x;
            const char* glyph_start = text.data();
            const char* const text_end = text.data() + text.size();
            int glyph_index = 0;
            while (glyph_start < text_end)
            {
                unsigned int codepoint;
                const int glyph_size = ImTextCharFromUtf8(&codepoint, glyph_start, text_end);
                if (glyph_size <= 0)
                    break;
                static_cast<void>(codepoint);

                const char* const glyph_end = glyph_start + glyph_size;
                const float blend =
                        (std::sin(animation_offset + direction * static_cast<float>(glyph_index)
                                                             * gradient.animation_spread)
                         + 1.f)
                        * 0.5f;
                const auto color = gradient.top_left.value() * (1.f - blend) + gradient.top_right.value() * blend;
                draw_list->AddText({cursor_x, position.y}, Color{color}.to_im_color(), glyph_start, glyph_end);
                cursor_x += ImGui::CalcTextSize(glyph_start, glyph_end).x;
                glyph_start = glyph_end;
                ++glyph_index;
            }
            return;
        }

        const int vertex_start = draw_list->VtxBuffer.Size;
        draw_list->AddText(position.to_im_vec2(), IM_COL32_WHITE, text.data(), text.data() + text.size());

        const auto size = calc_text_size(text);
        const float width = std::max(size.x, 1.f);
        const float height = std::max(size.y, 1.f);
        for (int i = vertex_start; i < draw_list->VtxBuffer.Size; ++i)
        {
            auto& vertex = draw_list->VtxBuffer[i];
            const float x = std::clamp((vertex.pos.x - position.x) / width, 0.f, 1.f);
            const float y = std::clamp((vertex.pos.y - position.y) / height, 0.f, 1.f);
            const auto top = gradient.top_left.value() * (1.f - x) + gradient.top_right.value() * x;
            const auto bottom = gradient.bottom_left.value() * (1.f - x) + gradient.bottom_right.value() * x;
            const auto color = top * (1.f - y) + bottom * y;
            const auto alpha = static_cast<int>(((vertex.col >> IM_COL32_A_SHIFT) & 0xff) * color.w);
            vertex.col = IM_COL32(static_cast<int>(color.x * 255.f), static_cast<int>(color.y * 255.f),
                                  static_cast<int>(color.z * 255.f), alpha);
        }
    }
    [[nodiscard]]
    Vector2<float> ImguiHudRenderer::calc_text_size(const std::string_view& text)
    {
        return Vector2<float>::from_im_vec2(ImGui::CalcTextSize(text.data()));
    }
} // namespace omath::hud
#endif // OMATH_IMGUI_INTEGRATION
