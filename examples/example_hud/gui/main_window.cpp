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
        // Entity position
        float entity_x = 550.f, entity_top_y = 150.f, entity_bottom_y = 450.f;
        float box_ratio = 4.f;

        // Box
        omath::Color box_color = {1.f, 1.f, 1.f, 1.f};
        omath::Color box_fill  = {0.f, 0.f, 0.f, 0.f};
        float box_thickness = 1.f;
        float corner_ratio = 0.2f;
        bool show_box = true, show_cornered_box = true;

        // Bars
        omath::Color bar_color         = {0.f, 1.f, 0.f, 1.f};
        omath::Color bar_bg_color      = {0.f, 0.f, 0.f, 0.5f};
        omath::Color bar_outline_color = {0.f, 0.f, 0.f, 1.f};
        float bar_width = 4.f, bar_value = 0.75f, bar_offset = 5.f;
        bool show_right_bar = true, show_left_bar = true;
        bool show_top_bar = true,   show_bottom_bar = true;

        // Labels
        float label_offset = 3.f;
        bool outlined = true;
        bool show_right_labels = true, show_left_labels = true;
        bool show_top_labels = true,   show_bottom_labels = true;
        bool show_centered_top = true, show_centered_bottom = true;

        // Dashed box
        omath::Color dash_color = omath::Color::from_rgba(255, 200, 0, 255);
        float dash_len = 8.f, dash_gap = 5.f, dash_thickness = 1.f;
        bool show_dashed_box = false;

        // Skeleton
        omath::Color skel_color = omath::Color::from_rgba(255, 255, 255, 200);
        float skel_thickness = 1.f;
        bool show_skeleton = false;

        // Snap line
        omath::Color snap_color = omath::Color::from_rgba(255, 50, 50, 255);
        float snap_width = 1.5f;
        bool show_snap = true;

        while (!glfwWindowShouldClose(m_window) && m_opened)
        {
            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            const auto* viewport = ImGui::GetMainViewport();
            ImGui::GetBackgroundDrawList()->AddRectFilled({}, viewport->Size, ImColor(30, 30, 30, 220));

            // ── Side panel ──────────────────────────────────────────────
            constexpr float panel_w = 280.f;
            ImGui::SetNextWindowPos({0.f, 0.f});
            ImGui::SetNextWindowSize({panel_w, viewport->Size.y});
            ImGui::Begin("Controls", &m_opened,
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            ImGui::PushItemWidth(160.f);

            if (ImGui::CollapsingHeader("Entity", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::SliderFloat("X##ent",      &entity_x,       100.f, viewport->Size.x - 100.f);
                ImGui::SliderFloat("Top Y",        &entity_top_y,   20.f,  entity_bottom_y - 20.f);
                ImGui::SliderFloat("Bottom Y",     &entity_bottom_y, entity_top_y + 20.f, viewport->Size.y - 20.f);
                ImGui::SliderFloat("Aspect ratio", &box_ratio,      1.f,   10.f);
            }

            if (ImGui::CollapsingHeader("Box", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Checkbox("Show box",          &show_box);
                ImGui::Checkbox("Show cornered box", &show_cornered_box);
                ImGui::Checkbox("Show dashed box",   &show_dashed_box);
                ImGui::ColorEdit4("Color##box",  reinterpret_cast<float*>(&box_color), ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Fill##box",   reinterpret_cast<float*>(&box_fill),  ImGuiColorEditFlags_NoInputs);
                ImGui::SliderFloat("Thickness",  &box_thickness, 0.5f, 5.f);
                ImGui::SliderFloat("Corner ratio",    &corner_ratio,    0.05f, 0.5f);
                ImGui::ColorEdit4("Dash color",        reinterpret_cast<float*>(&dash_color), ImGuiColorEditFlags_NoInputs);
                ImGui::SliderFloat("Dash length",      &dash_len,        2.f,  30.f);
                ImGui::SliderFloat("Dash gap",         &dash_gap,        1.f,  20.f);
                ImGui::SliderFloat("Dash thickness",   &dash_thickness,  0.5f,  5.f);
            }

            if (ImGui::CollapsingHeader("Bars", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::ColorEdit4("Color##bar",   reinterpret_cast<float*>(&bar_color),         ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("BG##bar",      reinterpret_cast<float*>(&bar_bg_color),      ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4("Outline##bar", reinterpret_cast<float*>(&bar_outline_color), ImGuiColorEditFlags_NoInputs);
                ImGui::SliderFloat("Width##bar",  &bar_width,  1.f, 20.f);
                ImGui::SliderFloat("Value##bar",  &bar_value,  0.f,  1.f);
                ImGui::SliderFloat("Offset##bar", &bar_offset, 1.f, 20.f);
                ImGui::Checkbox("Right bar",  &show_right_bar);  ImGui::SameLine();
                ImGui::Checkbox("Left bar",   &show_left_bar);
                ImGui::Checkbox("Top bar",    &show_top_bar);    ImGui::SameLine();
                ImGui::Checkbox("Bottom bar", &show_bottom_bar);
            }

            if (ImGui::CollapsingHeader("Labels", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Checkbox("Outlined", &outlined);
                ImGui::SliderFloat("Offset##lbl", &label_offset, 0.f, 15.f);
                ImGui::Checkbox("Right##lbl",          &show_right_labels);   ImGui::SameLine();
                ImGui::Checkbox("Left##lbl",           &show_left_labels);
                ImGui::Checkbox("Top##lbl",            &show_top_labels);     ImGui::SameLine();
                ImGui::Checkbox("Bottom##lbl",         &show_bottom_labels);
                ImGui::Checkbox("Centered top##lbl",   &show_centered_top);   ImGui::SameLine();
                ImGui::Checkbox("Centered bot##lbl",   &show_centered_bottom);
            }

            if (ImGui::CollapsingHeader("Skeleton"))
            {
                ImGui::Checkbox("Show##skel", &show_skeleton);
                ImGui::ColorEdit4("Color##skel", reinterpret_cast<float*>(&skel_color), ImGuiColorEditFlags_NoInputs);
                ImGui::SliderFloat("Thickness##skel", &skel_thickness, 0.5f, 5.f);
            }

            if (ImGui::CollapsingHeader("Snap Line"))
            {
                ImGui::Checkbox("Show##snap", &show_snap);
                ImGui::ColorEdit4("Color##snap", reinterpret_cast<float*>(&snap_color), ImGuiColorEditFlags_NoInputs);
                ImGui::SliderFloat("Width##snap", &snap_width, 0.5f, 5.f);
            }

            ImGui::PopItemWidth();
            ImGui::End();

            // ── Entity overlay ───────────────────────────────────────────
            omath::hud::EntityOverlay ent(
                {entity_x, entity_top_y}, {entity_x, entity_bottom_y},
                std::make_shared<omath::hud::ImguiHudRenderer>());

            if (show_box)
                ent.add_2d_box(box_color, box_fill, box_thickness);
            if (show_cornered_box)
                ent.add_cornered_2d_box(omath::Color::from_rgba(255, 0, 255, 255), box_fill, corner_ratio, box_thickness);
            if (show_dashed_box)
                ent.add_dashed_box(dash_color, dash_len, dash_gap, dash_thickness);

            if (show_right_bar)
                ent.add_right_bar(bar_color, bar_outline_color, bar_bg_color, bar_width, bar_value, bar_offset);
            if (show_left_bar)
                ent.add_left_bar(bar_color, bar_outline_color, bar_bg_color, bar_width, bar_value, bar_offset);
            if (show_top_bar)
                ent.add_top_bar(bar_color, bar_outline_color, bar_bg_color, bar_width, bar_value, bar_offset);
            if (show_bottom_bar)
                ent.add_bottom_bar(bar_color, bar_outline_color, bar_bg_color, bar_width, bar_value, bar_offset);

            if (show_right_labels)
            {
                ent.add_right_label({0.f, 1.f, 0.f, 1.f}, label_offset, outlined, "Health: {}/100", 100);
                ent.add_right_label({1.f, 0.f, 0.f, 1.f}, label_offset, outlined, "Shield: {}/125", 125);
                ent.add_right_label({1.f, 0.f, 1.f, 1.f}, label_offset, outlined, "*LOCKED*");
            }
            if (show_left_labels)
            {
                ent.add_left_label(omath::Color::from_rgba(255, 128, 0, 255), label_offset, outlined, "Armor: 75");
                ent.add_left_label(omath::Color::from_rgba(0, 200, 255, 255), label_offset, outlined, "Level: 42");
            }
            if (show_top_labels)
            {
                ent.add_top_label(omath::Color::from_rgba(255, 255, 0, 255), label_offset, outlined, "*SCOPED*");
                ent.add_top_label(omath::Color::from_rgba(255, 0,   0, 255), label_offset, outlined, "*BLEEDING*");
            }
            if (show_centered_top)
                ent.add_centered_top_label(omath::Color::from_rgba(0, 255, 255, 255), label_offset, outlined, "*VISIBLE*");
            if (show_centered_bottom)
                ent.add_centered_bottom_label(omath::Color::from_rgba(255, 255, 255, 255), label_offset, outlined, "PlayerName");
            if (show_bottom_labels)
                ent.add_bottom_label(omath::Color::from_rgba(200, 200, 0, 255), label_offset, outlined, "42m");

            if (show_skeleton)
                ent.add_skeleton(skel_color, skel_thickness);

            if (show_snap)
                ent.add_snap_line({viewport->Size.x / 2.f, viewport->Size.y}, snap_color, snap_width);

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
