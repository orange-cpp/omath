//
// Created by Vlad on 6/17/2025.
//

//
// Created by Orange on 11/11/2024.
//
#pragma once
#include <string_view>

struct GLFWwindow;

namespace imgui_desktop::gui
{
    class MainWindow
    {
    public:
        MainWindow(const std::string_view &caption, int width, int height);

        void Run();

    private:
        GLFWwindow* m_window;
        static bool m_canMoveWindow;
        bool m_opened = true;
    };
} // gui
// imgui_desktop