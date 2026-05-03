#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

#include "omath/hooks/hooks_manager.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
bool show_menu = true;

namespace
{
    struct frame_context
    {
        ID3D12Resource*             render_target = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle    = {};
    };

    bool g_initialized    = false;
    bool g_init_attempted = false;

    ID3D12Device*              g_device           = nullptr;
    ID3D12CommandQueue*        g_command_queue    = nullptr;
    IDXGISwapChain3*           g_swap_chain       = nullptr;
    ID3D12DescriptorHeap*      g_rtv_heap         = nullptr;
    ID3D12DescriptorHeap*      g_srv_heap         = nullptr;
    ID3D12GraphicsCommandList* g_command_list     = nullptr;
    ID3D12CommandAllocator*    g_command_allocator = nullptr;
    std::vector<frame_context> g_frames;

    void init(IDXGISwapChain* swap_chain)
    {
        g_init_attempted = true;

        if (FAILED(swap_chain->QueryInterface(IID_PPV_ARGS(&g_swap_chain))))
            return;

        if (FAILED(swap_chain->GetDevice(IID_PPV_ARGS(&g_device))))
            return;

        DXGI_SWAP_CHAIN_DESC desc{};
        swap_chain->GetDesc(&desc);
        const UINT buffer_count = desc.BufferCount;

        {
            D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
            heap_desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heap_desc.NumDescriptors = buffer_count;
            heap_desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            if (FAILED(g_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&g_srv_heap))))
                return;
        }
        {
            D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
            heap_desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            heap_desc.NumDescriptors = buffer_count;
            heap_desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            heap_desc.NodeMask       = 1;
            if (FAILED(g_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&g_rtv_heap))))
                return;
        }

        if (FAILED(g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                     IID_PPV_ARGS(&g_command_allocator))))
            return;

        g_frames.resize(buffer_count);
        const UINT rtv_size = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = g_rtv_heap->GetCPUDescriptorHandleForHeapStart();

        for (UINT i = 0; i < buffer_count; ++i)
        {
            g_frames[i].rtv_handle = rtv_handle;
            if (FAILED(swap_chain->GetBuffer(i, IID_PPV_ARGS(&g_frames[i].render_target))))
                return;
            g_device->CreateRenderTargetView(g_frames[i].render_target, nullptr, rtv_handle);
            rtv_handle.ptr += rtv_size;
        }

        if (FAILED(g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                g_command_allocator, nullptr,
                                                IID_PPV_ARGS(&g_command_list))))
            return;
        g_command_list->Close();

        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::GetIO().LogFilename = nullptr;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplWin32_Init(desc.OutputWindow);
        ImGui_ImplDX12_Init(g_device, static_cast<int>(buffer_count),
                             desc.BufferDesc.Format, g_srv_heap,
                             g_srv_heap->GetCPUDescriptorHandleForHeapStart(),
                             g_srv_heap->GetGPUDescriptorHandleForHeapStart());
        ImGui_ImplDX12_CreateDeviceObjects();

        auto& mgr = omath::hooks::HooksManager::get();
        mgr.set_on_wnd_proc([](HWND h, UINT msg, WPARAM wp, LPARAM lp) -> std::optional<LRESULT> {
            if (!show_menu)
                return std::nullopt;

            ImGui_ImplWin32_WndProcHandler(h, msg, wp, lp);
            return true;
        });
        mgr.hook_wnd_proc(desc.OutputWindow);

        g_initialized = true;
    }

    void on_execute_command_lists(ID3D12CommandQueue* queue, UINT, ID3D12CommandList* const*)
    {
        if (!g_command_queue)
            g_command_queue = queue;
    }

    void on_present(IDXGISwapChain* swap_chain, UINT, UINT)
    {
        if (!g_initialized)
        {
            if (!g_init_attempted && g_command_queue)
                init(swap_chain);
            return;
        }

        if (!g_command_queue)
            return;

        if (GetAsyncKeyState(VK_INSERT) & 1)
            show_menu = !show_menu;

        if (!show_menu)
            return;

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::GetIO().MouseDrawCursor = true;
        ImGui::ShowDemoWindow();
        ImGui::EndFrame();

        const UINT buf_idx = g_swap_chain->GetCurrentBackBufferIndex();
        auto& fc = g_frames[buf_idx];

        g_command_allocator->Reset();
        g_command_list->Reset(g_command_allocator, nullptr);

        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource   = fc.render_target;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
        g_command_list->ResourceBarrier(1, &barrier);
        g_command_list->OMSetRenderTargets(1, &fc.rtv_handle, FALSE, nullptr);
        g_command_list->SetDescriptorHeaps(1, &g_srv_heap);

        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_command_list);

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
        g_command_list->ResourceBarrier(1, &barrier);
        g_command_list->Close();

        ID3D12CommandList* cmd_lists[] = {g_command_list};
        g_command_queue->ExecuteCommandLists(1, cmd_lists);
    }

    void release_dx12_resources()
    {
        for (auto& fc : g_frames)
        {
            if (fc.render_target) { fc.render_target->Release(); fc.render_target = nullptr; }
        }
        g_frames.clear();
        if (g_command_allocator) { g_command_allocator->Release(); g_command_allocator = nullptr; }
        if (g_command_list)      { g_command_list->Release();      g_command_list      = nullptr; }
        if (g_srv_heap)          { g_srv_heap->Release();          g_srv_heap          = nullptr; }
        if (g_rtv_heap)          { g_rtv_heap->Release();          g_rtv_heap          = nullptr; }
        if (g_swap_chain)        { g_swap_chain->Release();        g_swap_chain        = nullptr; }
        if (g_device)            { g_device->Release();            g_device            = nullptr; }
    }
} // namespace

BOOL WINAPI DllMain(HINSTANCE h_instance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(h_instance);
        CreateThread(nullptr, 0, [](LPVOID) -> DWORD
        {
            while (!GetModuleHandle("d3d12.dll"))
                Sleep(100);

            auto& mgr = omath::hooks::HooksManager::get();
            mgr.set_on_present(on_present);
            mgr.set_on_execute_command_lists(on_execute_command_lists);
            mgr.hook_dx12();
            return 0;
        }, nullptr, 0, nullptr);
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        auto& mgr = omath::hooks::HooksManager::get();
        mgr.unhook_wnd_proc();
        mgr.unhook_dx12();

        if (g_initialized)
        {
            ImGui_ImplDX12_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }
        release_dx12_resources();
    }
    return TRUE;
}
