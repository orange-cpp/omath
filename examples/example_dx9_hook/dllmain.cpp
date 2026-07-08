#include <Windows.h>
#include <d3d9.h>
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include "omath/hooks/hooks_manager.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace
{
    bool g_initialized    = false;
    bool g_init_attempted = false;

    void init(IDirect3DDevice9* device)
    {
        g_init_attempted = true;

        D3DDEVICE_CREATION_PARAMETERS params{};
        if (FAILED(device->GetCreationParameters(&params)))
            return;

        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::GetIO().LogFilename = nullptr;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

        ImGui_ImplWin32_Init(params.hFocusWindow);
        ImGui_ImplDX9_Init(device);

        auto& mgr = omath::hooks::HooksManager::get();
        mgr.set_on_wnd_proc([](HWND h, UINT msg, WPARAM wp, LPARAM lp) -> std::optional<LRESULT> {
            if (ImGui_ImplWin32_WndProcHandler(h, msg, wp, lp))
                return 0;
            return std::nullopt;
        });
        std::ignore = mgr.hook_wnd_proc(params.hFocusWindow);

        g_initialized = true;
    }

    void on_present(IDirect3DDevice9* device, const RECT*, const RECT*, HWND, const RGNDATA*)
    {
        if (!g_initialized)
        {
            if (!g_init_attempted)
                init(device);
            return;
        }

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize({300.f, 80.f}, ImGuiCond_Once);
        ImGui::SetNextWindowPos({10.f, 10.f}, ImGuiCond_Once);
        ImGui::Begin("omath | DX9 hook");
        ImGui::Text("Hook active");
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::End();

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }

    void on_reset(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*)
    {
        if (g_initialized)
            ImGui_ImplDX9_InvalidateDeviceObjects();
    }
} // namespace

BOOL WINAPI DllMain(HINSTANCE h_instance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(h_instance);
        CreateThread(nullptr, 0, [](LPVOID) -> DWORD
        {
            while (!GetModuleHandle("d3d9.dll"))
                Sleep(100);

            auto& mgr = omath::hooks::HooksManager::get();
            mgr.set_on_dx9_present(on_present);
            mgr.set_on_dx9_reset(on_reset);
            std::ignore = mgr.hook_dx9();
            return 0;
        }, nullptr, 0, nullptr);
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        auto& mgr = omath::hooks::HooksManager::get();
        mgr.unhook_wnd_proc();
        mgr.unhook_dx9();

        if (g_initialized)
        {
            ImGui_ImplDX9_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }
    }
    return TRUE;
}
