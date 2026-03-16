//
// Created by orange on 13.03.2026.
//
#pragma once
#include "canvas_box.hpp"
#include "entity_overlay_widgets.hpp"
#include "hud_renderer_interface.hpp"
#include "omath/linear_algebra/vector2.hpp"
#include "omath/utility/color.hpp"
#include <memory>
#include <string_view>

namespace omath::hud
{
    class EntityOverlay final
    {
    public:
        EntityOverlay(const Vector2<float>& top, const Vector2<float>& bottom,
                      const std::shared_ptr<HudRendererInterface>& renderer);

        // ── Boxes ────────────────────────────────────────────────────────
        EntityOverlay& add_2d_box(const Color& box_color, const Color& fill_color = Color{0.f, 0.f, 0.f, 0.f},
                                  float thickness = 1.f);

        EntityOverlay& add_cornered_2d_box(const Color& box_color, const Color& fill_color = Color{0.f, 0.f, 0.f, 0.f},
                                           float corner_ratio_len = 0.2f, float thickness = 1.f);

        EntityOverlay& add_dashed_box(const Color& color, float dash_len = 8.f, float gap_len = 5.f,
                                      float thickness = 1.f);

        // ── Bars ─────────────────────────────────────────────────────────
        EntityOverlay& add_right_bar(const Color& color, const Color& outline_color, const Color& bg_color, float width,
                                     float ratio, float offset = 5.f);

        EntityOverlay& add_left_bar(const Color& color, const Color& outline_color, const Color& bg_color, float width,
                                    float ratio, float offset = 5.f);

        EntityOverlay& add_top_bar(const Color& color, const Color& outline_color, const Color& bg_color, float height,
                                   float ratio, float offset = 5.f);

        EntityOverlay& add_bottom_bar(const Color& color, const Color& outline_color, const Color& bg_color,
                                      float height, float ratio, float offset = 5.f);

        EntityOverlay& add_right_dashed_bar(const Color& color, const Color& outline_color, const Color& bg_color,
                                            float width, float ratio, float dash_len, float gap_len,
                                            float offset = 5.f);

        EntityOverlay& add_left_dashed_bar(const Color& color, const Color& outline_color, const Color& bg_color,
                                           float width, float ratio, float dash_len, float gap_len, float offset = 5.f);

        EntityOverlay& add_top_dashed_bar(const Color& color, const Color& outline_color, const Color& bg_color,
                                          float height, float ratio, float dash_len, float gap_len, float offset = 5.f);

        EntityOverlay& add_bottom_dashed_bar(const Color& color, const Color& outline_color, const Color& bg_color,
                                             float height, float ratio, float dash_len, float gap_len,
                                             float offset = 5.f);

        // ── Labels ───────────────────────────────────────────────────────
        EntityOverlay& add_right_label(const Color& color, float offset, bool outlined, const std::string_view& text);

        EntityOverlay& add_left_label(const Color& color, float offset, bool outlined, const std::string_view& text);

        EntityOverlay& add_top_label(const Color& color, float offset, bool outlined, std::string_view text);

        EntityOverlay& add_bottom_label(const Color& color, float offset, bool outlined, std::string_view text);

        EntityOverlay& add_centered_top_label(const Color& color, float offset, bool outlined,
                                              const std::string_view& text);

        EntityOverlay& add_centered_bottom_label(const Color& color, float offset, bool outlined,
                                                 const std::string_view& text);

        template<typename... Args>
        EntityOverlay& add_right_label(const Color& color, const float offset, const bool outlined, std::format_string<Args...> fmt,
                                       Args&&... args)
        {
            return add_right_label(color, offset, outlined,
                                   std::string_view{std::vformat(fmt.get(), std::make_format_args(args...))});
        }

        template<typename... Args>
        EntityOverlay& add_left_label(const Color& color, const float offset, const bool outlined, std::format_string<Args...> fmt,
                                      Args&&... args)
        {
            return add_left_label(color, offset, outlined,
                                  std::string_view{std::vformat(fmt.get(), std::make_format_args(args...))});
        }

        template<typename... Args>
        EntityOverlay& add_top_label(const Color& color, const float offset, const bool outlined, std::format_string<Args...> fmt,
                                     Args&&... args)
        {
            return add_top_label(color, offset, outlined,
                                 std::string_view{std::vformat(fmt.get(), std::make_format_args(args...))});
        }

        template<typename... Args>
        EntityOverlay& add_bottom_label(const Color& color, const float offset, const bool outlined,
                                        std::format_string<Args...> fmt, Args&&... args)
        {
            return add_bottom_label(color, offset, outlined,
                                    std::string_view{std::vformat(fmt.get(), std::make_format_args(args...))});
        }

        template<typename... Args>
        EntityOverlay& add_centered_top_label(const Color& color, const float offset, const bool outlined,
                                              std::format_string<Args...> fmt, Args&&... args)
        {
            return add_centered_top_label(color, offset, outlined,
                                          std::string_view{std::vformat(fmt.get(), std::make_format_args(args...))});
        }

        template<typename... Args>
        EntityOverlay& add_centered_bottom_label(const Color& color, const float offset, const bool outlined,
                                                 std::format_string<Args...> fmt, Args&&... args)
        {
            return add_centered_bottom_label(color, offset, outlined,
                                             std::string_view{std::vformat(fmt.get(), std::make_format_args(args...))});
        }

        // ── Spacers ─────────────────────────────────────────────────────
        EntityOverlay& add_right_space_vertical(float size);
        EntityOverlay& add_right_space_horizontal(float size);
        EntityOverlay& add_left_space_vertical(float size);
        EntityOverlay& add_left_space_horizontal(float size);
        EntityOverlay& add_top_space_vertical(float size);
        EntityOverlay& add_top_space_horizontal(float size);
        EntityOverlay& add_bottom_space_vertical(float size);
        EntityOverlay& add_bottom_space_horizontal(float size);

        // ── Progress rings ──────────────────────────────────────────────
        EntityOverlay& add_right_progress_ring(const Color& color, const Color& bg, float radius, float ratio,
                                               float thickness = 2.f, float offset = 5.f, int segments = 0);
        EntityOverlay& add_left_progress_ring(const Color& color, const Color& bg, float radius, float ratio,
                                              float thickness = 2.f, float offset = 5.f, int segments = 0);
        EntityOverlay& add_top_progress_ring(const Color& color, const Color& bg, float radius, float ratio,
                                             float thickness = 2.f, float offset = 5.f, int segments = 0);
        EntityOverlay& add_bottom_progress_ring(const Color& color, const Color& bg, float radius, float ratio,
                                                float thickness = 2.f, float offset = 5.f, int segments = 0);

        // ── Misc ─────────────────────────────────────────────────────────
        EntityOverlay& add_snap_line(const Vector2<float>& start_pos, const Color& color, float width);

        EntityOverlay& add_skeleton(const Color& color, float thickness = 1.f);

        // ── Declarative interface ─────────────────────────────────────────
        /// Pass any combination of widget:: descriptor structs (and std::optional<W>
        /// from when()) to render them all in declaration order.
        template<typename... Widgets>
        EntityOverlay& contents(Widgets&&... widgets)
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

        void dispatch(const widget::Box& box);
        void dispatch(const widget::CorneredBox& cornered_box);
        void dispatch(const widget::DashedBox& dashed_box);
        void dispatch(const widget::RightSide& right_side);
        void dispatch(const widget::LeftSide& left_side);
        void dispatch(const widget::TopSide& top_side);
        void dispatch(const widget::BottomSide& bottom_side);
        void dispatch(const widget::Skeleton& skeleton);
        void dispatch(const widget::SnapLine& snap_line);
        void draw_progress_ring(const Vector2<float>& center, const widget::ProgressRing& ring);
        void draw_outlined_text(const Vector2<float>& position, const Color& color, const std::string_view& text);
        void draw_dashed_line(const Vector2<float>& from, const Vector2<float>& to, const Color& color, float dash_len,
                              float gap_len, float thickness) const;
        void draw_dashed_fill(const Vector2<float>& origin, const Vector2<float>& step_dir,
                              const Vector2<float>& perp_dir, float full_len, float filled_len, const Color& fill_color,
                              const Color& split_color, float dash_len, float gap_len) const;

        CanvasBox m_canvas;
        Vector2<float> m_text_cursor_right;
        Vector2<float> m_text_cursor_top;
        Vector2<float> m_text_cursor_bottom;
        Vector2<float> m_text_cursor_left;
        std::shared_ptr<HudRendererInterface> m_renderer;
    };
} // namespace omath::hud
