#include "omath/hooks/hooks_manager.hpp"
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>
#include <mutex>
#include <tuple>
#include <vector>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
bool show_menu = true;

namespace
{
    constexpr UINT srv_heap_size = 64;

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
    ID3D12CommandQueue* g_pending_command_queue = nullptr;
    IDXGISwapChain3* g_swap_chain = nullptr;
    IUnknown* g_swap_chain_identity = nullptr;
    ID3D12DescriptorHeap* g_rtv_heap = nullptr;
    ID3D12DescriptorHeap* g_srv_heap = nullptr;
    ID3D12GraphicsCommandList* g_command_list = nullptr;
    ID3D12Fence* g_fence = nullptr;
    HANDLE g_fence_event = nullptr;
    UINT64 g_fence_value = 0;
    UINT g_srv_descriptor_size = 0;
    bool g_command_queue_selected = false;
    std::mutex g_command_queue_mutex;
    std::vector<UINT> g_free_srv_indices;
    std::vector<frame_context> g_frames;

    void allocate_srv_descriptor(ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* cpu_handle,
                                 D3D12_GPU_DESCRIPTOR_HANDLE* gpu_handle)
    {
        IM_ASSERT(!g_free_srv_indices.empty());
        const UINT index = g_free_srv_indices.back();
        g_free_srv_indices.pop_back();

        *cpu_handle = g_srv_heap->GetCPUDescriptorHandleForHeapStart();
        *gpu_handle = g_srv_heap->GetGPUDescriptorHandleForHeapStart();
        cpu_handle->ptr += static_cast<SIZE_T>(index) * g_srv_descriptor_size;
        gpu_handle->ptr += static_cast<UINT64>(index) * g_srv_descriptor_size;
    }

    void free_srv_descriptor(ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
                             D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
    {
        const D3D12_CPU_DESCRIPTOR_HANDLE cpu_start = g_srv_heap->GetCPUDescriptorHandleForHeapStart();
        const D3D12_GPU_DESCRIPTOR_HANDLE gpu_start = g_srv_heap->GetGPUDescriptorHandleForHeapStart();
        const UINT cpu_index = static_cast<UINT>((cpu_handle.ptr - cpu_start.ptr) / g_srv_descriptor_size);
        const UINT gpu_index = static_cast<UINT>((gpu_handle.ptr - gpu_start.ptr) / g_srv_descriptor_size);
        IM_ASSERT(cpu_index == gpu_index && cpu_index < srv_heap_size);
        g_free_srv_indices.push_back(cpu_index);
    }

    void remember_direct_command_queue(ID3D12CommandQueue* queue)
    {
        if (queue->GetDesc().Type != D3D12_COMMAND_LIST_TYPE_DIRECT)
            return;

        std::scoped_lock lock(g_command_queue_mutex);
        if (g_command_queue_selected || g_pending_command_queue == queue)
            return;

        queue->AddRef();
        if (g_pending_command_queue)
            g_pending_command_queue->Release();
        g_pending_command_queue = queue;
    }

    ID3D12CommandQueue* take_command_queue_for_device(ID3D12Device* device)
    {
        std::scoped_lock lock(g_command_queue_mutex);
        if (!g_pending_command_queue)
            return nullptr;

        ID3D12Device* queue_device = nullptr;
        if (FAILED(g_pending_command_queue->GetDevice(IID_PPV_ARGS(&queue_device))))
            return nullptr;

        const bool matches = queue_device == device;
        queue_device->Release();
        if (!matches)
            return nullptr;

        ID3D12CommandQueue* queue = g_pending_command_queue;
        g_pending_command_queue = nullptr;
        g_command_queue_selected = true;
        return queue;
    }

    bool is_target_swap_chain(IDXGISwapChain* swap_chain)
    {
        if (!g_swap_chain_identity)
            return false;

        IUnknown* identity = nullptr;
        if (FAILED(swap_chain->QueryInterface(IID_PPV_ARGS(&identity))))
            return false;

        const bool matches = identity == g_swap_chain_identity;
        identity->Release();
        return matches;
    }

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
        IDXGISwapChain3* swap_chain3 = nullptr;
        if (FAILED(swap_chain->QueryInterface(IID_PPV_ARGS(&swap_chain3))))
            return;

        ID3D12Device* device = nullptr;
        if (FAILED(swap_chain->GetDevice(IID_PPV_ARGS(&device))))
        {
            swap_chain3->Release();
            return;
        }

        IUnknown* swap_chain_identity = nullptr;
        if (FAILED(swap_chain->QueryInterface(IID_PPV_ARGS(&swap_chain_identity))))
        {
            device->Release();
            swap_chain3->Release();
            return;
        }

        DXGI_SWAP_CHAIN_DESC desc{};
        if (FAILED(swap_chain->GetDesc(&desc)))
        {
            swap_chain_identity->Release();
            device->Release();
            swap_chain3->Release();
            return;
        }

        ID3D12CommandQueue* command_queue = take_command_queue_for_device(device);
        if (!command_queue)
        {
            swap_chain_identity->Release();
            device->Release();
            swap_chain3->Release();
            return;
        }

        g_init_attempted = true;
        g_swap_chain = swap_chain3;
        g_swap_chain_identity = swap_chain_identity;
        g_device = device;
        g_command_queue = command_queue;
        const UINT buffer_count = desc.BufferCount;

        {
            D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
            heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heap_desc.NumDescriptors = srv_heap_size;
            heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            if (FAILED(g_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&g_srv_heap))))
                return;

            g_srv_descriptor_size = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            g_free_srv_indices.reserve(srv_heap_size);
            for (UINT i = srv_heap_size; i > 0; --i)
                g_free_srv_indices.push_back(i - 1);
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

        ImGui_ImplDX12_InitInfo init_info{};
        init_info.Device = g_device;
        init_info.CommandQueue = g_command_queue;
        init_info.NumFramesInFlight = static_cast<int>(buffer_count);
        init_info.RTVFormat = desc.BufferDesc.Format;
        init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
        init_info.SrvDescriptorHeap = g_srv_heap;
        init_info.SrvDescriptorAllocFn = allocate_srv_descriptor;
        init_info.SrvDescriptorFreeFn = free_srv_descriptor;
        ImGui_ImplDX12_Init(&init_info);
        ImGui_ImplDX12_CreateDeviceObjects();

        auto& mgr = omath::hooks::HooksManager::get();
        mgr.set_on_wnd_proc(
                [](HWND h, UINT msg, WPARAM wp, LPARAM lp) -> std::optional<LRESULT>
                {
                    if (!show_menu)
                        return std::nullopt;

                    if (ImGui_ImplWin32_WndProcHandler(h, msg, wp, lp))
                        return true;

                    return std::nullopt;
                });
        std::ignore = mgr.hook_wnd_proc(desc.OutputWindow);

        g_initialized = true;
    }

    void on_execute_command_lists(ID3D12CommandQueue* queue, UINT, ID3D12CommandList* const*)
    {
        // The most recently used DIRECT queue is the best available proxy for the swap chain's presentation queue.
        remember_direct_command_queue(queue);
    }

    bool ensure_initialized(IDXGISwapChain* swap_chain)
    {
        if (g_initialized)
            return true;

        if (!g_init_attempted)
            init(swap_chain);

        return false;
    }

    bool ensure_render_targets(IDXGISwapChain* swap_chain)
    {
        if (!g_frames.empty() && g_rtv_heap)
            return true;

        return create_render_targets(swap_chain);
    }

    bool ensure_command_list()
    {
        if (g_command_list)
            return true;

        return create_command_objects();
    }

    bool ensure_sync_ready()
    {
        if (g_fence)
            return true;

        return create_sync_objects();
    }

    bool ensure_present_resources(IDXGISwapChain* swap_chain)
    {
        if (!ensure_initialized(swap_chain) || !g_command_queue)
            return false;

        return ensure_render_targets(swap_chain) && ensure_command_list() && ensure_sync_ready();
    }

    bool begin_imgui_frame()
    {
        if (GetAsyncKeyState(VK_INSERT) & 1)
            show_menu = !show_menu;

        if (!show_menu)
            return false;

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::GetIO().MouseDrawCursor = true;
        ImGui::ShowDemoWindow();
        ImGui::EndFrame();
        return true;
    }

    frame_context* current_frame_context()
    {
        const UINT buf_idx = g_swap_chain->GetCurrentBackBufferIndex();
        if (buf_idx >= g_frames.size())
            return nullptr;

        return &g_frames[buf_idx];
    }

    bool reset_overlay_command_list(frame_context& fc)
    {
        wait_for_frame(fc);

        // Both resets depend on wait_for_frame(); otherwise DLSSG/Streamline can observe invalid GPU work.
        if (FAILED(fc.command_allocator->Reset()))
            return false;

        if (FAILED(g_command_list->Reset(fc.command_allocator, nullptr)))
            return false;

        return true;
    }

    void transition_render_target(frame_context& fc, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
    {
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = fc.render_target;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = before;
        barrier.Transition.StateAfter = after;
        g_command_list->ResourceBarrier(1, &barrier);
    }

    void record_overlay_commands(frame_context& fc)
    {
        transition_render_target(fc, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        g_command_list->OMSetRenderTargets(1, &fc.rtv_handle, FALSE, nullptr);
        g_command_list->SetDescriptorHeaps(1, &g_srv_heap);

        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_command_list);

        transition_render_target(fc, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    }

    bool submit_overlay_commands(frame_context& fc)
    {
        if (FAILED(g_command_list->Close()))
            return false;

        ID3D12CommandList* cmd_lists[] = {g_command_list};
        g_command_queue->ExecuteCommandLists(1, cmd_lists);
        return signal_frame(fc);
    }

    void on_present(IDXGISwapChain* swap_chain, UINT, UINT)
    {
        if (g_initialized && !is_target_swap_chain(swap_chain))
            return;

        if (!ensure_present_resources(swap_chain) || !begin_imgui_frame())
            return;

        frame_context* fc = current_frame_context();
        if (!fc || !reset_overlay_command_list(*fc))
            return;

        record_overlay_commands(*fc);
        std::ignore = submit_overlay_commands(*fc);
    }

    void on_resize_buffers(IDXGISwapChain* swap_chain, UINT, UINT, UINT, DXGI_FORMAT, UINT)
    {
        if (!is_target_swap_chain(swap_chain))
            return;

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
        g_srv_descriptor_size = 0;
        g_free_srv_indices.clear();
        if (g_swap_chain)
        {
            g_swap_chain->Release();
            g_swap_chain = nullptr;
        }
        if (g_swap_chain_identity)
        {
            g_swap_chain_identity->Release();
            g_swap_chain_identity = nullptr;
        }
        if (g_device)
        {
            g_device->Release();
            g_device = nullptr;
        }
        if (g_command_queue)
        {
            g_command_queue->Release();
            g_command_queue = nullptr;
        }

        std::scoped_lock lock(g_command_queue_mutex);
        if (g_pending_command_queue)
        {
            g_pending_command_queue->Release();
            g_pending_command_queue = nullptr;
        }
        g_command_queue_selected = false;
    }
} // namespace

BOOL WINAPI DllMain(HINSTANCE h_instance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(h_instance);
        std::thread(
                []()
                {
                    while (!GetModuleHandle("d3d12.dll"))
                        Sleep(100);

                    auto& mgr = omath::hooks::HooksManager::get();
                    mgr.set_on_present(on_present);
                    mgr.set_on_resize_buffers(on_resize_buffers);
                    mgr.set_on_execute_command_lists(on_execute_command_lists);
                    std::ignore = mgr.hook_dx12();
                    return 0;
                })
                .detach();
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
