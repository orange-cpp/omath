//
// Created by Orange on 11/11/2024.
//

#include "main_window.hpp"
#include "omath/hud/renderer_realizations/imgui_renderer.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
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

        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);
        m_window = glfwCreateWindow(width, height, caption.data(), nullptr, nullptr);

        glfwMakeContextCurrent(m_window);

        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = {0.f, 0.f, 0.f, 0.f};

        ImGui::GetStyle().AntiAliasedLines = false;
        ImGui::GetStyle().AntiAliasedFill = false;

        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 130");
    }

    void MainWindow::Run()
    {
        omath::Color box_color = {0.f, 0.f, 0.f, 1.f};
        omath::Color box_fill = {0.f, 0.f, 0.f, 0.f};
        omath::Color bar_color = {0.f, 1.f, 0.f, 1.f};
        omath::Color bar_bg_color = {0.f, 0.f, 0.f, 0.0f};
        omath::Color bar_outline_color = {0.f, 0.f, 0.f, 1.f};
        float bar_width = 3.f;
        float bar_value = 1.f;

        while (!glfwWindowShouldClose(m_window) && m_opened)
        {
            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::GetBackgroundDrawList()->AddRectFilled({}, ImGui::GetMainViewport()->Size, ImColor(40, 40, 40, 200));

            ImGui::Begin("OHUD Showcase", &m_opened,
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration);
            {
                ImGui::SetWindowPos({});
                ImGui::SetWindowSize(ImGui::GetMainViewport()->Size);

                ImGui::ColorEdit4("Box", reinterpret_cast<float*>(&box_color), ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Box fill", reinterpret_cast<float*>(&box_fill), ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Bar", reinterpret_cast<float*>(&bar_color), ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Bar Background", reinterpret_cast<float*>(&bar_bg_color),
                                  ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Bar Outline", reinterpret_cast<float*>(&bar_outline_color),
                                  ImGuiColorEditFlags_NoInputs);

                ImGui::PushItemWidth(100.f);
                ImGui::SliderFloat("Bar Width", &bar_width, 1.f, 20.f);
                ImGui::SliderFloat("Bar Value", &bar_value, 0.f, 1.f);
                ImGui::PopItemWidth();
                ImGui::End();
            }

            omath::hud::EntityOverlay ent({400.f, 100.f}, {400.f, 400.f}, std::make_shared<omath::hud::ImguiHudRenderer>());

            ent.add_2d_box(box_color, box_fill, 1.f);
            ent.add_cornered_2d_box(omath::Color::from_rgba(255, 0, 255, 255), box_fill);
            ent.add_right_bar(bar_color, bar_outline_color, bar_bg_color, bar_width, bar_value);
            ent.add_left_bar(bar_color, bar_outline_color, bar_bg_color, bar_width, bar_value);
            ent.add_top_bar(bar_color, bar_outline_color, bar_bg_color, bar_width, bar_value);
            ent.add_right_label({0.f, 1.f, 0.f, 1.f}, 3, true, "Health: {}/100", 100);
            ent.add_right_label({1.f, 0.f, 0.f, 1.f}, 3, true, "Shield: {}/125", 125);
            ent.add_right_label({1.f, 0.f, 1.f, 1.f}, 3, true, "*LOCKED*");

            ent.add_top_label(omath::Color::from_rgba(255, 255, 0, 255), 3, true, "*SCOPED*");
            ent.add_top_label(omath::Color::from_rgba(255, 0, 0, 255), 3, true, "*BLEEDING*");
            ent.add_snap_line(omath::Vector2<float>{400, 600}, omath::Color::from_rgba(255, 0, 0, 255), 2.f);
            ImGui::Render();

            int display_w, display_h;

            glfwGetFramebufferSize(m_window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);

            glClearColor(0.f, 0.f, 0.f, 0.f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(m_window);
        }
        glfwDestroyWindow(m_window);
    }
} // namespace imgui_desktop::gui
// imgui_desktop