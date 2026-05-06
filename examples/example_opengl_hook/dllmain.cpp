#include "omath/hooks/hooks_manager.hpp"
#include <Windows.h>
#include <chrono>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>
#include <optional>
#include <thread>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace
{
    bool g_initialized = false;
    bool g_init_attempted = false;
    bool g_show_menu = true;

    constexpr auto g_module_wait_delay = std::chrono::milliseconds{100};

    void init(HDC hdc)
    {
        g_init_attempted = true;

        const HWND hwnd = WindowFromDC(hdc);
        if (!hwnd)
            return;

        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::GetIO().LogFilename = nullptr;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplOpenGL3_Init();

        auto& mgr = omath::hooks::HooksManager::get();
        mgr.set_on_wnd_proc(
                [](HWND h, UINT msg, WPARAM wp, LPARAM lp) -> std::optional<LRESULT>
                {
                    if (!g_show_menu)
                        return std::nullopt;

                    if (ImGui_ImplWin32_WndProcHandler(h, msg, wp, lp))
                        return 0;
                    return std::nullopt;
                });
        (void)mgr.hook_wnd_proc(hwnd);

        g_initialized = true;
    }

    void on_swap_buffers(HDC hdc)
    {
        if (!g_initialized)
        {
            if (!g_init_attempted)
                init(hdc);
            return;
        }

        if (GetAsyncKeyState(VK_INSERT) & 1)
            g_show_menu = !g_show_menu;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (g_show_menu)
        {
            ImGui::SetNextWindowSize({300.f, 100.f}, ImGuiCond_Once);
            ImGui::SetNextWindowPos({10.f, 10.f}, ImGuiCond_Once);
            ImGui::Begin("omath | OpenGL hook");
            ImGui::Text("Hook active");
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("INSERT toggles this window");
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void hook_when_opengl_is_loaded()
    {
        while (!GetModuleHandle("opengl32.dll"))
            std::this_thread::sleep_for(g_module_wait_delay);

        auto& mgr = omath::hooks::HooksManager::get();
        mgr.set_on_opengl_swap_buffers(on_swap_buffers);
        (void)mgr.hook_opengl();
    }
} // namespace

BOOL WINAPI DllMain(HINSTANCE h_instance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(h_instance);
        std::thread{hook_when_opengl_is_loaded}.detach();
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        auto& mgr = omath::hooks::HooksManager::get();
        mgr.unhook_wnd_proc();
        mgr.unhook_opengl();

        if (g_initialized)
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }
    }
    return true;
}
