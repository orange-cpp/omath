#include "omath/hooks/hooks_manager.hpp"

#ifdef OMATH_ENABLE_HOOKING
#include <cstring>

namespace
{
    class DummyWindow final
    {
        WNDCLASSEX m_window_class{};
        HWND m_window_handle = nullptr;

    public:
        DummyWindow()
        {
            m_window_class.cbSize        = sizeof(WNDCLASSEX);
            m_window_class.style         = CS_HREDRAW | CS_VREDRAW;
            m_window_class.lpfnWndProc   = DefWindowProc;
            m_window_class.hInstance     = GetModuleHandle(nullptr);
            m_window_class.lpszClassName = "OM";
            RegisterClassEx(&m_window_class);
            m_window_handle = CreateWindow(m_window_class.lpszClassName, "Dummy", WS_OVERLAPPEDWINDOW,
                                           0, 0, 100, 100, nullptr, nullptr, m_window_class.hInstance, nullptr);
        }
        ~DummyWindow()
        {
            if (m_window_handle)
                DestroyWindow(m_window_handle);
            UnregisterClass(m_window_class.lpszClassName, m_window_class.hInstance);
        }
        [[nodiscard]] HWND handle() const { return m_window_handle; }
        [[nodiscard]] bool valid()  const { return m_window_handle != nullptr; }
    };

    // Method counts per interface (vtable slots including inherited ones)
    constexpr std::size_t k_device_methods           = 44;
    constexpr std::size_t k_command_queue_methods     = 19;
    constexpr std::size_t k_command_allocator_methods = 9;
    constexpr std::size_t k_command_list_methods      = 60;
    constexpr std::size_t k_swap_chain_methods        = 18;
    constexpr std::size_t k_total_methods = k_device_methods + k_command_queue_methods +
                                            k_command_allocator_methods + k_command_list_methods +
                                            k_swap_chain_methods; // 150

    // Base offsets in the combined table
    constexpr std::size_t k_cmd_queue_base  = k_device_methods; // 44
    constexpr std::size_t k_swap_chain_base = k_device_methods + k_command_queue_methods +
                                              k_command_allocator_methods + k_command_list_methods; // 132

    // IDXGISwapChain vtable: Present=8, ResizeBuffers=13 (from IUnknown base)
    // ID3D12CommandQueue vtable: ExecuteCommandLists=8 (from IUnknown base)
    constexpr std::size_t k_present_index          = k_swap_chain_base + 8;   // 140
    constexpr std::size_t k_resize_buffers_index   = k_swap_chain_base + 13;  // 145
    constexpr std::size_t k_execute_cmd_lists_index = k_cmd_queue_base + 8;   // 52
} // namespace

namespace omath::hooks
{
    HooksManager& HooksManager::get()
    {
        static HooksManager obj;
        return obj;
    }

    HooksManager::~HooksManager()
    {
        unhook_wnd_proc();
        unhook_dx12();
    }

    bool HooksManager::build_vtable()
    {
        const DummyWindow window;
        if (!window.valid())
            return false;

        const HMODULE d3d12_module = GetModuleHandle("d3d12.dll");
        const HMODULE dxgi_module  = GetModuleHandle("dxgi.dll");
        if (!d3d12_module || !dxgi_module)
            return false;

        using create_dxgi_factory_fn = HRESULT(__stdcall*)(REFIID, void**);
        using d3d12_create_device_fn = HRESULT(__stdcall*)(IUnknown*, D3D_FEATURE_LEVEL, REFIID, void**);

        const auto create_dxgi_factory = reinterpret_cast<create_dxgi_factory_fn>(
            GetProcAddress(dxgi_module, "CreateDXGIFactory"));
        const auto d3d12_create_device = reinterpret_cast<d3d12_create_device_fn>(
            GetProcAddress(d3d12_module, "D3D12CreateDevice"));

        if (!create_dxgi_factory || !d3d12_create_device)
            return false;

        IDXGIFactory* factory = nullptr;
        if (FAILED(create_dxgi_factory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory))))
            return false;

        IDXGIAdapter* adapter = nullptr;
        if (factory->EnumAdapters(0, &adapter) == DXGI_ERROR_NOT_FOUND)
        {
            factory->Release();
            return false;
        }

        ID3D12Device* device = nullptr;
        if (FAILED(d3d12_create_device(adapter, D3D_FEATURE_LEVEL_11_0,
                                       __uuidof(ID3D12Device), reinterpret_cast<void**>(&device))))
        {
            adapter->Release();
            factory->Release();
            return false;
        }
        adapter->Release();

        D3D12_COMMAND_QUEUE_DESC queue_desc{};
        queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        ID3D12CommandQueue* command_queue = nullptr;
        if (FAILED(device->CreateCommandQueue(&queue_desc, __uuidof(ID3D12CommandQueue),
                                              reinterpret_cast<void**>(&command_queue))))
        {
            device->Release();
            factory->Release();
            return false;
        }

        ID3D12CommandAllocator* command_allocator = nullptr;
        if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                   __uuidof(ID3D12CommandAllocator),
                                                   reinterpret_cast<void**>(&command_allocator))))
        {
            command_queue->Release();
            device->Release();
            factory->Release();
            return false;
        }

        ID3D12GraphicsCommandList* command_list = nullptr;
        if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator, nullptr,
                                              __uuidof(ID3D12GraphicsCommandList),
                                              reinterpret_cast<void**>(&command_list))))
        {
            command_allocator->Release();
            command_queue->Release();
            device->Release();
            factory->Release();
            return false;
        }

        DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
        swap_chain_desc.BufferDesc.Width       = 100;
        swap_chain_desc.BufferDesc.Height      = 100;
        swap_chain_desc.BufferDesc.RefreshRate = {60, 1};
        swap_chain_desc.BufferDesc.Format      = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_chain_desc.SampleDesc             = {1, 0};
        swap_chain_desc.BufferUsage            = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.BufferCount            = 2;
        swap_chain_desc.OutputWindow           = window.handle();
        swap_chain_desc.Windowed               = TRUE;
        swap_chain_desc.SwapEffect             = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swap_chain_desc.Flags                  = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        IDXGISwapChain* swap_chain = nullptr;
        if (FAILED(factory->CreateSwapChain(command_queue, &swap_chain_desc, &swap_chain)))
        {
            command_list->Release();
            command_allocator->Release();
            command_queue->Release();
            device->Release();
            factory->Release();
            return false;
        }

        m_vtable.resize(k_total_methods);

        const auto copy_vtable = [](uintptr_t* dst, void* com_obj, std::size_t count)
        {
            std::memcpy(dst, *reinterpret_cast<uintptr_t**>(com_obj), count * sizeof(uintptr_t));
        };

        copy_vtable(m_vtable.data(),                                                                           device,           k_device_methods);
        copy_vtable(m_vtable.data() + k_device_methods,                                                        command_queue,    k_command_queue_methods);
        copy_vtable(m_vtable.data() + k_device_methods + k_command_queue_methods,                              command_allocator, k_command_allocator_methods);
        copy_vtable(m_vtable.data() + k_device_methods + k_command_queue_methods + k_command_allocator_methods, command_list,    k_command_list_methods);
        copy_vtable(m_vtable.data() + k_swap_chain_base,                                                       swap_chain,       k_swap_chain_methods);

        swap_chain->Release();
        command_list->Release();
        command_allocator->Release();
        command_queue->Release();
        device->Release();
        factory->Release();

        return true;
    }

    bool HooksManager::hook_dx12()
    {
        std::unique_lock lock(m_mutex);
        if (m_is_dx12_hooked)
            return true;

        if (!build_vtable())
            return false;

        m_present_hook = safetyhook::create_inline(
            reinterpret_cast<void*>(m_vtable[k_present_index]),
            reinterpret_cast<void*>(&present_detour));

        m_resize_buffers_hook = safetyhook::create_inline(
            reinterpret_cast<void*>(m_vtable[k_resize_buffers_index]),
            reinterpret_cast<void*>(&resize_buffers_detour));

        m_execute_command_lists_hook = safetyhook::create_inline(
            reinterpret_cast<void*>(m_vtable[k_execute_cmd_lists_index]),
            reinterpret_cast<void*>(&execute_command_lists_detour));

        if (!m_present_hook || !m_resize_buffers_hook || !m_execute_command_lists_hook)
        {
            m_present_hook               = {};
            m_resize_buffers_hook        = {};
            m_execute_command_lists_hook = {};
            return false;
        }

        m_is_dx12_hooked = true;
        return true;
    }

    void HooksManager::unhook_dx12()
    {
        std::unique_lock lock(m_mutex);
        m_present_hook               = {};
        m_resize_buffers_hook        = {};
        m_execute_command_lists_hook = {};
        m_vtable.clear();
        m_is_dx12_hooked = false;
    }

    void HooksManager::set_on_present(present_callback callback)
    {
        std::unique_lock lock(m_mutex);
        m_present_cb = std::move(callback);
    }

    void HooksManager::set_on_resize_buffers(resize_buffers_callback callback)
    {
        std::unique_lock lock(m_mutex);
        m_resize_buffers_cb = std::move(callback);
    }

    void HooksManager::set_on_execute_command_lists(execute_command_lists_callback callback)
    {
        std::unique_lock lock(m_mutex);
        m_execute_command_lists_cb = std::move(callback);
    }

    bool HooksManager::hook_wnd_proc(HWND hwnd)
    {
        std::unique_lock lock(m_mutex);
        if (m_is_wnd_proc_hooked)
            return true;

        const auto prev = reinterpret_cast<WNDPROC>(
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&wnd_proc_detour)));
        if (!prev)
            return false;

        m_hooked_hwnd       = hwnd;
        m_original_wndproc  = prev;
        m_is_wnd_proc_hooked = true;
        return true;
    }

    void HooksManager::unhook_wnd_proc()
    {
        std::unique_lock lock(m_mutex);
        if (!m_is_wnd_proc_hooked)
            return;

        SetWindowLongPtr(m_hooked_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_original_wndproc));
        m_hooked_hwnd        = nullptr;
        m_original_wndproc   = nullptr;
        m_is_wnd_proc_hooked = false;
    }

    void HooksManager::set_on_wnd_proc(wnd_proc_callback callback)
    {
        std::unique_lock lock(m_mutex);
        m_wnd_proc_cb = std::move(callback);
    }

    // Detour implementations: copy callback under shared lock, call it unlocked,
    // then call original. This avoids a deadlock if the callback itself calls set_on_*().
    HRESULT __stdcall HooksManager::present_detour(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags)
    {
        auto& mgr = get();
        present_callback cb;
        {
            std::shared_lock lock(mgr.m_mutex);
            cb = mgr.m_present_cb;
        }
        if (cb)
            cb(p_swap_chain, sync_interval, flags);
        return mgr.m_present_hook.call<HRESULT>(p_swap_chain, sync_interval, flags);
    }

    HRESULT __stdcall HooksManager::resize_buffers_detour(IDXGISwapChain* p_swap_chain, UINT buffer_count,
                                                          UINT width, UINT height, DXGI_FORMAT new_format,
                                                          UINT swap_chain_flags)
    {
        auto& mgr = get();
        resize_buffers_callback cb;
        {
            std::shared_lock lock(mgr.m_mutex);
            cb = mgr.m_resize_buffers_cb;
        }
        if (cb)
            cb(p_swap_chain, buffer_count, width, height, new_format, swap_chain_flags);
        return mgr.m_resize_buffers_hook.call<HRESULT>(p_swap_chain, buffer_count, width, height, new_format,
                                                       swap_chain_flags);
    }

    void __stdcall HooksManager::execute_command_lists_detour(ID3D12CommandQueue* p_command_queue,
                                                              UINT num_command_lists,
                                                              ID3D12CommandList* const* pp_command_lists)
    {
        auto& mgr = get();
        execute_command_lists_callback cb;
        {
            std::shared_lock lock(mgr.m_mutex);
            cb = mgr.m_execute_command_lists_cb;
        }
        if (cb)
            cb(p_command_queue, num_command_lists, pp_command_lists);
        mgr.m_execute_command_lists_hook.call<void>(p_command_queue, num_command_lists, pp_command_lists);
    }

    LRESULT __stdcall HooksManager::wnd_proc_detour(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
    {
        auto& mgr = get();
        wnd_proc_callback cb;
        WNDPROC original;
        {
            std::shared_lock lock(mgr.m_mutex);
            cb       = mgr.m_wnd_proc_cb;
            original = mgr.m_original_wndproc;
        }
        if (cb)
        {
            if (const auto result = cb(hwnd, msg, w_param, l_param))
                return *result;
        }
        return CallWindowProc(original, hwnd, msg, w_param, l_param);
    }
} // namespace omath::hooks

#else // !OMATH_ENABLE_HOOKING

namespace omath::hooks
{
    HooksManager& HooksManager::get()
    {
        static HooksManager obj;
        return obj;
    }
    HooksManager::~HooksManager() = default;
} // namespace omath::hooks

#endif
