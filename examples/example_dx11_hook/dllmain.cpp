#include "omath/hooks/hooks_manager.hpp"
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <thread>
#include <tuple>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace
{
    bool g_initialized = false;
    bool g_init_attempted = false;

    ID3D11Device* g_device = nullptr;
    ID3D11DeviceContext* g_context = nullptr;
    ID3D11RenderTargetView* g_render_target_view = nullptr;

    [[nodiscard]]
    bool create_render_target(IDXGISwapChain* swap_chain)
    {
        ID3D11Texture2D* back_buffer = nullptr;
        if (FAILED(swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer))))
            return false;

        D3D11_RENDER_TARGET_VIEW_DESC desc{};
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        const HRESULT result = g_device->CreateRenderTargetView(back_buffer, &desc, &g_render_target_view);
        back_buffer->Release();
        return SUCCEEDED(result);
    }

    void release_render_target()
    {
        if (g_context)
            g_context->OMSetRenderTargets(0, nullptr, nullptr);

        if (g_render_target_view)
        {
            g_render_target_view->Release();
            g_render_target_view = nullptr;
        }

        if (g_context)
            g_context->Flush();
    }

    void init(IDXGISwapChain* swap_chain)
    {
        g_init_attempted = true;

        if (FAILED(swap_chain->GetDevice(IID_PPV_ARGS(&g_device))))
            return;

        g_device->GetImmediateContext(&g_context);

        DXGI_SWAP_CHAIN_DESC desc{};
        std::ignore = swap_chain->GetDesc(&desc);

        std::ignore = create_render_target(swap_chain);

        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::GetIO().LogFilename = nullptr;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

        ImGui_ImplWin32_Init(desc.OutputWindow);
        ImGui_ImplDX11_Init(g_device, g_context);

        auto& mgr = omath::hooks::HooksManager::get();
        mgr.set_on_wnd_proc(
                [](const HWND h, const UINT msg, const WPARAM wp, const LPARAM lp) -> std::optional<LRESULT>
                {
                    if (ImGui_ImplWin32_WndProcHandler(h, msg, wp, lp))
                        return 0;
                    return std::nullopt;
                });
        std::ignore = mgr.hook_wnd_proc(desc.OutputWindow);

        g_initialized = true;
    }

    void on_present(IDXGISwapChain* swap_chain, UINT, UINT)
    {
        if (!g_initialized)
        {
            if (!g_init_attempted)
                init(swap_chain);
            return;
        }

        if (!g_render_target_view && !create_render_target(swap_chain))
            return;

        ID3D11RenderTargetView* previous_render_target_views[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
        ID3D11DepthStencilView* previous_depth_stencil_view = nullptr;
        g_context->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, previous_render_target_views,
                                      &previous_depth_stencil_view);

        g_context->OMSetRenderTargets(1, &g_render_target_view, nullptr);

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::GetIO().MouseDrawCursor = true;
        ImGui::ShowDemoWindow();

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_context->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, previous_render_target_views,
                                      previous_depth_stencil_view);
        for (ID3D11RenderTargetView* render_target_view : previous_render_target_views)
        {
            if (render_target_view)
                render_target_view->Release();
        }
        if (previous_depth_stencil_view)
            previous_depth_stencil_view->Release();
    }

    void on_resize_buffers(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)
    {
        release_render_target();
    }
} // namespace

BOOL WINAPI DllMain(const HINSTANCE h_instance, const DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(h_instance);
        std::thread(
                []()
                {
                    while (!GetModuleHandle("d3d11.dll"))
                        Sleep(100);

                    auto& mgr = omath::hooks::HooksManager::get();
                    mgr.set_on_present(on_present);
                    mgr.set_on_resize_buffers(on_resize_buffers);
                    std::ignore = mgr.hook_dx11();
                    return 0;
                })
                .detach();
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        auto& mgr = omath::hooks::HooksManager::get();
        mgr.unhook_wnd_proc();
        mgr.unhook_dx11();

        if (g_initialized)
        {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }

        release_render_target();
        if (g_context)
        {
            g_context->Release();
            g_context = nullptr;
        }
        if (g_device)
        {
            g_device->Release();
            g_device = nullptr;
        }
    }
    return TRUE;
}
