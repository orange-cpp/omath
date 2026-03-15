//
// Created by Orange on 11/11/2024.
//
#pragma once
#include <omath/hud/entity_overlay.hpp>
#include <omath/utility/color.hpp>
#include <string_view>

struct GLFWwindow;

namespace imgui_desktop::gui
{
    class MainWindow
    {
    public:
        MainWindow(const std::string_view& caption, int width, int height);
        void Run();

    private:
        void draw_controls();
        void draw_overlay();
        void present();

        GLFWwindow* m_window = nullptr;
        static bool m_canMoveWindow;
        bool m_opened = true;

        // Entity
        float m_entity_x = 550.f, m_entity_top_y = 150.f, m_entity_bottom_y = 450.f;

        // Box
        omath::Color m_box_color{1.f, 1.f, 1.f, 1.f};
        omath::Color m_box_fill{0.f, 0.f, 0.f, 0.f};
        float m_box_thickness = 1.f, m_corner_ratio = 0.2f;
        bool m_show_box = true, m_show_cornered_box = true, m_show_dashed_box = false;

        // Dashed box
        omath::Color m_dash_color = omath::Color::from_rgba(255, 200, 0, 255);
        float m_dash_len = 8.f, m_dash_gap = 5.f, m_dash_thickness = 1.f;

        // Bars
        omath::Color m_bar_color{0.f, 1.f, 0.f, 1.f};
        omath::Color m_bar_bg_color{0.f, 0.f, 0.f, 0.5f};
        omath::Color m_bar_outline_color{0.f, 0.f, 0.f, 1.f};
        float m_bar_width = 4.f, m_bar_value = 0.75f, m_bar_offset = 5.f;
        bool m_show_right_bar = true,         m_show_left_bar = true;
        bool m_show_top_bar = true,           m_show_bottom_bar = true;
        bool m_show_right_dashed_bar = false, m_show_left_dashed_bar = false;
        bool m_show_top_dashed_bar = false,   m_show_bottom_dashed_bar = false;
        float m_bar_dash_len = 6.f, m_bar_dash_gap = 4.f;

        // Labels
        float m_label_offset = 3.f;
        bool m_outlined = true;
        bool m_show_right_labels = true, m_show_left_labels = true;
        bool m_show_top_labels = true,   m_show_bottom_labels = true;
        bool m_show_centered_top = true, m_show_centered_bottom = true;

        // Skeleton
        omath::Color m_skel_color = omath::Color::from_rgba(255, 255, 255, 200);
        float m_skel_thickness = 1.f;
        bool m_show_skeleton = false;

        // Snap line
        omath::Color m_snap_color = omath::Color::from_rgba(255, 50, 50, 255);
        float m_snap_width = 1.5f;
        bool m_show_snap = true;
    };
} // namespace imgui_desktop::gui
