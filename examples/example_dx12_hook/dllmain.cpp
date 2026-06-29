#include "omath/hooks/hooks_manager.hpp"
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>
#include <tuple>
#include <vector>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
bool show_menu = true;

namespace
{
    struct frame_context
    {
        ID3D12Resource* render_target = nullptr;
        // Each back buffer gets its own allocator because allocators cannot be reset while GPU work uses them.
        ID3D12CommandAllocator* command_allocator = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = {};
        UINT64 fence_value = 0;
    };

    bool g_initialized = false;
    bool g_init_attempted = false;

    ID3D12Device* g_device = nullptr;
    ID3D12CommandQueue* g_command_queue = nullptr;
    IDXGISwapChain3* g_swap_chain = nullptr;
    ID3D12DescriptorHeap* g_rtv_heap = nullptr;
    ID3D12DescriptorHeap* g_srv_heap = nullptr;
    ID3D12GraphicsCommandList* g_command_list = nullptr;
    ID3D12Fence* g_fence = nullptr;
    HANDLE g_fence_event = nullptr;
    UINT64 g_fence_value = 0;
    std::vector<frame_context> g_frames;

    // This fence tracks only the overlay work submitted by this DLL, not the game's whole frame.
    bool create_sync_objects()
    {
        if (g_fence)
            return true;

        if (FAILED(g_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence))))
            return false;

        g_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!g_fence_event)
        {
            g_fence->Release();
            g_fence = nullptr;
            return false;
        }

        g_fence_value = 0;
        return true;
    }

    bool wait_for_fence_value(UINT64 fence_value)
    {
        if (!g_fence || !g_fence_event || fence_value == 0 || g_fence->GetCompletedValue() >= fence_value)
            return true;

        if (FAILED(g_fence->SetEventOnCompletion(fence_value, g_fence_event)))
            return false;

        WaitForSingleObject(g_fence_event, INFINITE);
        return true;
    }

    void wait_for_frame(frame_context& fc)
    {
        // The current back buffer's allocator is safe to reset only after its previous overlay pass completes.
        if (wait_for_fence_value(fc.fence_value))
            fc.fence_value = 0;
    }

    void wait_for_gpu()
    {
        // ResizeBuffers and shutdown must not release back buffers still referenced by queued overlay commands.
        if (!g_command_queue || !g_fence || !g_fence_event)
            return;

        const UINT64 fence_value = ++g_fence_value;
        if (FAILED(g_command_queue->Signal(g_fence, fence_value)))
            return;

        if (wait_for_fence_value(fence_value))
        {
            for (auto& fc : g_frames)
                fc.fence_value = 0;
        }
    }

    bool signal_frame(frame_context& fc)
    {
        if (!g_command_queue || !g_fence)
            return false;

        const UINT64 fence_value = ++g_fence_value;
        if (FAILED(g_command_queue->Signal(g_fence, fence_value)))
            return false;

        fc.fence_value = fence_value;
        return true;
    }

    void release_sync_objects()
    {
        if (g_fence_event)
        {
            CloseHandle(g_fence_event);
            g_fence_event = nullptr;
        }
        if (g_fence)
        {
            g_fence->Release();
            g_fence = nullptr;
        }
        g_fence_value = 0;
    }

    void release_frame_contexts()
    {
        for (auto& fc : g_frames)
        {
            if (fc.render_target)
            {
                fc.render_target->Release();
                fc.render_target = nullptr;
            }
            if (fc.command_allocator)
            {
                fc.command_allocator->Release();
                fc.command_allocator = nullptr;
            }
            fc.fence_value = 0;
        }
        g_frames.clear();

        if (g_rtv_heap)
        {
            g_rtv_heap->Release();
            g_rtv_heap = nullptr;
        }
    }

    void release_command_objects()
    {
        if (g_command_list)
        {
            g_command_list->Release();
            g_command_list = nullptr;
        }
    }

    bool create_command_objects()
    {
        if (g_frames.empty() || !g_frames[0].command_allocator)
            return false;

        if (FAILED(g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frames[0].command_allocator,
                                               nullptr, IID_PPV_ARGS(&g_command_list))))
        {
            release_command_objects();
            return false;
        }

        g_command_list->Close();
        return true;
    }

    bool create_render_targets(IDXGISwapChain* swap_chain)
    {
        // These references must be released before IDXGISwapChain::ResizeBuffers reaches the original function.
        DXGI_SWAP_CHAIN_DESC desc{};
        if (FAILED(swap_chain->GetDesc(&desc)))
            return false;

        const UINT buffer_count = desc.BufferCount;

        D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
        heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        heap_desc.NumDescriptors = buffer_count;
        heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        heap_desc.NodeMask = 1;
        if (FAILED(g_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&g_rtv_heap))))
            return false;

        g_frames.resize(buffer_count);
        const UINT rtv_size = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = g_rtv_heap->GetCPUDescriptorHandleForHeapStart();

        for (UINT i = 0; i < buffer_count; ++i)
        {
            g_frames[i].rtv_handle = rtv_handle;
            if (FAILED(g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                        IID_PPV_ARGS(&g_frames[i].command_allocator))))
            {
                release_frame_contexts();
                return false;
            }
            if (FAILED(swap_chain->GetBuffer(i, IID_PPV_ARGS(&g_frames[i].render_target))))
            {
                release_frame_contexts();
                return false;
            }
            g_device->CreateRenderTargetView(g_frames[i].render_target, nullptr, rtv_handle);
            rtv_handle.ptr += rtv_size;
        }

        return true;
    }

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
            heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heap_desc.NumDescriptors = buffer_count;
            heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            if (FAILED(g_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&g_srv_heap))))
                return;
        }

        if (!create_render_targets(swap_chain))
            return;

        if (!create_command_objects())
            return;

        if (!create_sync_objects())
            return;

        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::GetIO().LogFilename = nullptr;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplWin32_Init(desc.OutputWindow);
        ImGui_ImplDX12_Init(g_device, static_cast<int>(buffer_count), desc.BufferDesc.Format, g_srv_heap,
                            g_srv_heap->GetCPUDescriptorHandleForHeapStart(),
                            g_srv_heap->GetGPUDescriptorHandleForHeapStart());
        ImGui_ImplDX12_CreateDeviceObjects();

        auto& mgr = omath::hooks::HooksManager::get();
        mgr.set_on_wnd_proc(
                [](HWND h, UINT msg, WPARAM wp, LPARAM lp) -> std::optional<LRESULT>
                {
                    if (!show_menu)
                        return std::nullopt;

                    ImGui_ImplWin32_WndProcHandler(h, msg, wp, lp);
                    return true;
                });
        std::ignore = mgr.hook_wnd_proc(desc.OutputWindow);

        g_initialized = true;
    }

    void on_execute_command_lists(ID3D12CommandQueue* queue, UINT, ID3D12CommandList* const*)
    {
        // The overlay records DIRECT command lists; executing them on COPY/COMPUTE queues can remove the device.
        if (!g_command_queue)
        {
            const D3D12_COMMAND_QUEUE_DESC desc = queue->GetDesc();
            if (desc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT)
                g_command_queue = queue;
        }
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

        if (g_frames.empty() || !g_rtv_heap)
        {
            if (!create_render_targets(swap_chain))
                return;
        }

        if (!g_command_list)
        {
            if (!create_command_objects())
                return;
        }

        if (!g_fence)
        {
            if (!create_sync_objects())
                return;
        }

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
        if (buf_idx >= g_frames.size())
            return;

        auto& fc = g_frames[buf_idx];

        wait_for_frame(fc);

        // Both resets depend on wait_for_frame(); otherwise DLSSG/Streamline can observe invalid GPU work.
        if (FAILED(fc.command_allocator->Reset()))
            return;

        if (FAILED(g_command_list->Reset(fc.command_allocator, nullptr)))
            return;

        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = fc.render_target;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        g_command_list->ResourceBarrier(1, &barrier);
        g_command_list->OMSetRenderTargets(1, &fc.rtv_handle, FALSE, nullptr);
        g_command_list->SetDescriptorHeaps(1, &g_srv_heap);

        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_command_list);

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        g_command_list->ResourceBarrier(1, &barrier);
        if (FAILED(g_command_list->Close()))
            return;

        ID3D12CommandList* cmd_lists[] = {g_command_list};
        g_command_queue->ExecuteCommandLists(1, cmd_lists);
        std::ignore = signal_frame(fc);
    }

    void on_resize_buffers(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)
    {
        wait_for_gpu();
        release_command_objects();
        release_frame_contexts();
    }

    void release_dx12_resources()
    {
        wait_for_gpu();
        release_command_objects();
        release_frame_contexts();
        release_sync_objects();
        if (g_srv_heap)
        {
            g_srv_heap->Release();
            g_srv_heap = nullptr;
        }
        if (g_swap_chain)
        {
            g_swap_chain->Release();
            g_swap_chain = nullptr;
        }
        if (g_device)
        {
            g_device->Release();
            g_device = nullptr;
        }
    }
} // namespace

BOOL WINAPI DllMain(HINSTANCE h_instance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(h_instance);
        CreateThread(
                nullptr, 0,
                [](LPVOID) -> DWORD
                {
                    while (!GetModuleHandle("d3d12.dll"))
                        Sleep(100);

                    auto& mgr = omath::hooks::HooksManager::get();
                    mgr.set_on_present(on_present);
                    mgr.set_on_resize_buffers(on_resize_buffers);
                    mgr.set_on_execute_command_lists(on_execute_command_lists);
                    std::ignore = mgr.hook_dx12();
                    return 0;
                },
                nullptr, 0, nullptr);
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
