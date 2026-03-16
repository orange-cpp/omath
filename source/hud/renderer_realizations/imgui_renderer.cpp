//
// Created by orange on 13.03.2026.
//
#include "omath/hud/renderer_realizations/imgui_renderer.hpp"

#ifdef OMATH_IMGUI_INTEGRATION
#include <imgui.h>

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

    void ImguiHudRenderer::add_circle(const Vector2<float>& center, const float radius, const Color& color,
                                       const float thickness, const int segments)
    {
        ImGui::GetBackgroundDrawList()->AddCircle(center.to_im_vec2(), radius, color.to_im_color(), segments, thickness);
    }

    void ImguiHudRenderer::add_filled_circle(const Vector2<float>& center, const float radius, const Color& color,
                                              const int segments)
    {
        ImGui::GetBackgroundDrawList()->AddCircleFilled(center.to_im_vec2(), radius, color.to_im_color(), segments);
    }

    void ImguiHudRenderer::add_arc(const Vector2<float>& center, const float radius, const float a_min, const float a_max,
                                    const Color& color, const float thickness, const int segments)
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
    [[nodiscard]]
    Vector2<float> ImguiHudRenderer::calc_text_size(const std::string_view& text)
    {
        return Vector2<float>::from_im_vec2(ImGui::CalcTextSize(text.data()));
    }
} // namespace omath::hud
#endif // OMATH_IMGUI_INTEGRATION
