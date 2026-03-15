//
// Created by Orange on 11/11/2024.
//
#include "main_window.hpp"
#include "omath/hud/renderer_realizations/imgui_renderer.hpp"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <omath/hud/entity_overlay.hpp>

namespace imgui_desktop::gui
{
    bool MainWindow::m_canMoveWindow = false;

    MainWindow::MainWindow(const std::string_view& caption, int width, int height)
    {
        if (!glfwInit())
            std::exit(EXIT_FAILURE);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);
        m_window = glfwCreateWindow(width, height, caption.data(), nullptr, nullptr);

        glfwMakeContextCurrent(m_window);

        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = {0.05f, 0.05f, 0.05f, 0.92f};
        ImGui::GetStyle().AntiAliasedLines = false;
        ImGui::GetStyle().AntiAliasedFill = false;
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 150");
    }

    void MainWindow::Run()
    {
        while (!glfwWindowShouldClose(m_window) && m_opened)
        {
            glfwPollEvents();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            const auto* vp = ImGui::GetMainViewport();
            ImGui::GetBackgroundDrawList()->AddRectFilled({}, vp->Size, ImColor(30, 30, 30, 220));

            draw_controls();
            draw_overlay();

            ImGui::Render();
            present();
        }
        glfwDestroyWindow(m_window);
    }

    void MainWindow::draw_controls()
    {
        const auto* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos({0.f, 0.f});
        ImGui::SetNextWindowSize({280.f, vp->Size.y});
        ImGui::Begin("Controls", &m_opened,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::PushItemWidth(160.f);

        if (ImGui::CollapsingHeader("Entity", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("X##ent", &m_entity_x, 100.f, vp->Size.x - 100.f);
            ImGui::SliderFloat("Top Y", &m_entity_top_y, 20.f, m_entity_bottom_y - 20.f);
            ImGui::SliderFloat("Bottom Y", &m_entity_bottom_y, m_entity_top_y + 20.f, vp->Size.y - 20.f);
        }

        if (ImGui::CollapsingHeader("Box", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Box##chk", &m_show_box);
            ImGui::SameLine();
            ImGui::Checkbox("Cornered", &m_show_cornered_box);
            ImGui::SameLine();
            ImGui::Checkbox("Dashed", &m_show_dashed_box);
            ImGui::ColorEdit4("Color##box", reinterpret_cast<float*>(&m_box_color), ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit4("Fill##box", reinterpret_cast<float*>(&m_box_fill), ImGuiColorEditFlags_NoInputs);
            ImGui::SliderFloat("Thickness", &m_box_thickness, 0.5f, 5.f);
            ImGui::SliderFloat("Corner ratio", &m_corner_ratio, 0.05f, 0.5f);
            ImGui::Separator();
            ImGui::ColorEdit4("Dash color", reinterpret_cast<float*>(&m_dash_color), ImGuiColorEditFlags_NoInputs);
            ImGui::SliderFloat("Dash length", &m_dash_len, 2.f, 30.f);
            ImGui::SliderFloat("Dash gap", &m_dash_gap, 1.f, 20.f);
            ImGui::SliderFloat("Dash thick", &m_dash_thickness, 0.5f, 5.f);
        }

        if (ImGui::CollapsingHeader("Bars", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::ColorEdit4("Color##bar", reinterpret_cast<float*>(&m_bar_color), ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit4("BG##bar", reinterpret_cast<float*>(&m_bar_bg_color), ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit4("Outline##bar", reinterpret_cast<float*>(&m_bar_outline_color),
                              ImGuiColorEditFlags_NoInputs);
            ImGui::SliderFloat("Width##bar", &m_bar_width, 1.f, 20.f);
            ImGui::SliderFloat("Value##bar", &m_bar_value, 0.f, 1.f);
            ImGui::SliderFloat("Offset##bar", &m_bar_offset, 1.f, 20.f);
            ImGui::Checkbox("Right##bar", &m_show_right_bar);
            ImGui::SameLine();
            ImGui::Checkbox("Left##bar", &m_show_left_bar);
            ImGui::Checkbox("Top##bar", &m_show_top_bar);
            ImGui::SameLine();
            ImGui::Checkbox("Bottom##bar", &m_show_bottom_bar);
            ImGui::Checkbox("Right dashed##bar", &m_show_right_dashed_bar);
            ImGui::SameLine();
            ImGui::Checkbox("Left dashed##bar", &m_show_left_dashed_bar);
            ImGui::Checkbox("Top dashed##bar", &m_show_top_dashed_bar);
            ImGui::SameLine();
            ImGui::Checkbox("Bot dashed##bar", &m_show_bottom_dashed_bar);
            ImGui::SliderFloat("Dash len##bar", &m_bar_dash_len, 2.f, 20.f);
            ImGui::SliderFloat("Dash gap##bar", &m_bar_dash_gap, 1.f, 15.f);
        }

        if (ImGui::CollapsingHeader("Labels", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Outlined", &m_outlined);
            ImGui::SliderFloat("Offset##lbl", &m_label_offset, 0.f, 15.f);
            ImGui::Checkbox("Right##lbl", &m_show_right_labels);
            ImGui::SameLine();
            ImGui::Checkbox("Left##lbl", &m_show_left_labels);
            ImGui::Checkbox("Top##lbl", &m_show_top_labels);
            ImGui::SameLine();
            ImGui::Checkbox("Bottom##lbl", &m_show_bottom_labels);
            ImGui::Checkbox("Ctr top##lbl", &m_show_centered_top);
            ImGui::SameLine();
            ImGui::Checkbox("Ctr bot##lbl", &m_show_centered_bottom);
        }

        if (ImGui::CollapsingHeader("Skeleton"))
        {
            ImGui::Checkbox("Show##skel", &m_show_skeleton);
            ImGui::ColorEdit4("Color##skel", reinterpret_cast<float*>(&m_skel_color), ImGuiColorEditFlags_NoInputs);
            ImGui::SliderFloat("Thick##skel", &m_skel_thickness, 0.5f, 5.f);
        }

        if (ImGui::CollapsingHeader("Snap Line"))
        {
            ImGui::Checkbox("Show##snap", &m_show_snap);
            ImGui::ColorEdit4("Color##snap", reinterpret_cast<float*>(&m_snap_color), ImGuiColorEditFlags_NoInputs);
            ImGui::SliderFloat("Width##snap", &m_snap_width, 0.5f, 5.f);
        }

        ImGui::PopItemWidth();
        ImGui::End();
    }

    void MainWindow::draw_overlay()
    {
        using namespace omath::hud::widget;
        using omath::hud::when;
        const auto* vp = ImGui::GetMainViewport();
        const Bar bar{m_bar_color, m_bar_outline_color, m_bar_bg_color, m_bar_width, m_bar_value, m_bar_offset};
        const DashedBar dbar{m_bar_color, m_bar_outline_color, m_bar_bg_color, m_bar_width,
                             m_bar_value, m_bar_dash_len,      m_bar_dash_gap, m_bar_offset};

        omath::hud::EntityOverlay({m_entity_x, m_entity_top_y}, {m_entity_x, m_entity_bottom_y},
                                  std::make_shared<omath::hud::ImguiHudRenderer>())
                .contents(
                        // ── Boxes ────────────────────────────────────────────────────
                        when(m_show_box, Box{m_box_color, m_box_fill, m_box_thickness}),
                        when(m_show_cornered_box, CorneredBox{omath::Color::from_rgba(255, 0, 255, 255), m_box_fill,
                                                              m_corner_ratio, m_box_thickness}),
                        when(m_show_dashed_box, DashedBox{m_dash_color, m_dash_len, m_dash_gap, m_dash_thickness}),

                        RightSide{
                                when(m_show_right_bar, bar),
                                when(m_show_right_dashed_bar, dbar),
                                when(m_show_right_labels,
                                     Label{{0.f, 1.f, 0.f, 1.f}, m_label_offset, m_outlined, "Health: 100/100"}),
                                when(m_show_right_labels,
                                     Label{{1.f, 0.f, 0.f, 1.f}, m_label_offset, m_outlined, "Shield: 125/125"}),
                                when(m_show_right_labels,
                                     Label{{1.f, 0.f, 1.f, 1.f}, m_label_offset, m_outlined, "*LOCKED*"}),
                        },
                        LeftSide{
                                when(m_show_left_bar, bar),
                                when(m_show_left_dashed_bar, dbar),
                                when(m_show_left_labels, Label{omath::Color::from_rgba(255, 128, 0, 255),
                                                               m_label_offset, m_outlined, "Armor: 75"}),
                                when(m_show_left_labels, Label{omath::Color::from_rgba(0, 200, 255, 255),
                                                               m_label_offset, m_outlined, "Level: 42"}),
                        },
                        TopSide{
                                when(m_show_top_bar, bar),
                                when(m_show_top_dashed_bar, dbar),
                                when(m_show_centered_top, Centered{Label{omath::Color::from_rgba(0, 255, 255, 255),
                                                                         m_label_offset, m_outlined, "*VISIBLE*"}}),
                                when(m_show_top_labels, Label{omath::Color::from_rgba(255, 255, 0, 255), m_label_offset,
                                                              m_outlined, "*SCOPED*"}),
                                when(m_show_top_labels, Label{omath::Color::from_rgba(255, 0, 0, 255), m_label_offset,
                                                              m_outlined, "*BLEEDING*"}),
                        },
                        BottomSide{
                                when(m_show_bottom_bar, bar),
                                when(m_show_bottom_dashed_bar, dbar),
                                when(m_show_centered_bottom, Centered{Label{omath::Color::from_rgba(255, 255, 255, 255),
                                                                            m_label_offset, m_outlined, "PlayerName"}}),
                                when(m_show_bottom_labels, Label{omath::Color::from_rgba(200, 200, 0, 255),
                                                                 m_label_offset, m_outlined, "42m"}),
                        },
                        when(m_show_skeleton, Skeleton{m_skel_color, m_skel_thickness}),
                        when(m_show_snap, SnapLine{{vp->Size.x / 2.f, vp->Size.y}, m_snap_color, m_snap_width}));
    }

    void MainWindow::present()
    {
        int w, h;
        glfwGetFramebufferSize(m_window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(m_window);
    }
} // namespace imgui_desktop::gui
