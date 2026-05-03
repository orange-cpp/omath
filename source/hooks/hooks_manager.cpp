#include "omath/hooks/hooks_manager.hpp"

#ifdef OMATH_ENABLE_HOOKING
#include <d3d11.h>

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

    void* vtable_fn(void* com_obj, std::size_t index)
    {
        return (*reinterpret_cast<void***>(com_obj))[index];
    }

    struct dx12_vtable_fns
    {
        void* present;
        void* resize_buffers;
        void* execute_command_lists;
    };

    // RAII wrapper so all early-return paths release COM objects automatically.
    struct dx12_com_objects
    {
        IDXGIFactory*              factory           = nullptr;
        ID3D12Device*              device            = nullptr;
        ID3D12CommandQueue*        command_queue     = nullptr;
        ID3D12CommandAllocator*    command_allocator = nullptr;
        ID3D12GraphicsCommandList* command_list      = nullptr;
        IDXGISwapChain*            swap_chain        = nullptr;

        dx12_com_objects() = default;
        dx12_com_objects(const dx12_com_objects&)            = delete;
        dx12_com_objects& operator=(const dx12_com_objects&) = delete;

        ~dx12_com_objects()
        {
            if (swap_chain)        swap_chain->Release();
            if (command_list)      command_list->Release();
            if (command_allocator) command_allocator->Release();
            if (command_queue)     command_queue->Release();
            if (device)            device->Release();
            if (factory)           factory->Release();
        }
    };

    std::optional<dx12_vtable_fns> read_dx12_vtable_fns(HWND hwnd)
    {
        using create_dxgi_factory_fn = HRESULT(__stdcall*)(REFIID, void**);
        using d3d12_create_device_fn = HRESULT(__stdcall*)(IUnknown*, D3D_FEATURE_LEVEL, REFIID, void**);

        const HMODULE d3d12_module = GetModuleHandle("d3d12.dll");
        const HMODULE dxgi_module  = GetModuleHandle("dxgi.dll");
        if (!d3d12_module || !dxgi_module)
            return std::nullopt;

        const auto create_dxgi_factory = reinterpret_cast<create_dxgi_factory_fn>(
            GetProcAddress(dxgi_module, "CreateDXGIFactory"));
        const auto d3d12_create_device = reinterpret_cast<d3d12_create_device_fn>(
            GetProcAddress(d3d12_module, "D3D12CreateDevice"));

        if (!create_dxgi_factory || !d3d12_create_device)
            return std::nullopt;

        dx12_com_objects objs;

        if (FAILED(create_dxgi_factory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&objs.factory))))
            return std::nullopt;

        IDXGIAdapter* adapter = nullptr;
        if (objs.factory->EnumAdapters(0, &adapter) == DXGI_ERROR_NOT_FOUND)
            return std::nullopt;

        const HRESULT device_hr = d3d12_create_device(adapter, D3D_FEATURE_LEVEL_11_0,
                                                       __uuidof(ID3D12Device),
                                                       reinterpret_cast<void**>(&objs.device));
        adapter->Release();
        if (FAILED(device_hr))
            return std::nullopt;

        D3D12_COMMAND_QUEUE_DESC queue_desc{};
        queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        if (FAILED(objs.device->CreateCommandQueue(&queue_desc, __uuidof(ID3D12CommandQueue),
                                                   reinterpret_cast<void**>(&objs.command_queue))))
            return std::nullopt;

        if (FAILED(objs.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                        __uuidof(ID3D12CommandAllocator),
                                                        reinterpret_cast<void**>(&objs.command_allocator))))
            return std::nullopt;

        if (FAILED(objs.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, objs.command_allocator,
                                                   nullptr, __uuidof(ID3D12GraphicsCommandList),
                                                   reinterpret_cast<void**>(&objs.command_list))))
            return std::nullopt;

        DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
        swap_chain_desc.BufferDesc.Width       = 100;
        swap_chain_desc.BufferDesc.Height      = 100;
        swap_chain_desc.BufferDesc.RefreshRate = {60, 1};
        swap_chain_desc.BufferDesc.Format      = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_chain_desc.SampleDesc             = {1, 0};
        swap_chain_desc.BufferUsage            = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.BufferCount            = 2;
        swap_chain_desc.OutputWindow           = hwnd;
        swap_chain_desc.Windowed               = TRUE;
        swap_chain_desc.SwapEffect             = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swap_chain_desc.Flags                  = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        if (FAILED(objs.factory->CreateSwapChain(objs.command_queue, &swap_chain_desc, &objs.swap_chain)))
            return std::nullopt;

        // objs destructor releases all COM objects after we capture the addresses.
        return dx12_vtable_fns{
            vtable_fn(objs.swap_chain, 8),    // IDXGISwapChain::Present
            vtable_fn(objs.swap_chain, 13),   // IDXGISwapChain::ResizeBuffers
            vtable_fn(objs.command_queue, 8), // ID3D12CommandQueue::ExecuteCommandLists
        };
    }
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
        unhook_dx9();
        unhook_dx11();
        unhook_dx12();
    }

    bool HooksManager::hook_dx9()
    {
        std::unique_lock lock(m_mutex);
        if (m_is_dx9_hooked)
            return true;

        const DummyWindow window;
        if (!window.valid())
            return false;

        const HMODULE d3d9_module = GetModuleHandle("d3d9.dll");
        if (!d3d9_module)
            return false;

        using direct3d_create9_fn = IDirect3D9*(__stdcall*)(UINT);
        const auto direct3d_create9 = reinterpret_cast<direct3d_create9_fn>(
            GetProcAddress(d3d9_module, "Direct3DCreate9"));
        if (!direct3d_create9)
            return false;

        IDirect3D9* d3d9 = direct3d_create9(D3D_SDK_VERSION);
        if (!d3d9)
            return false;

        D3DPRESENT_PARAMETERS pp{};
        pp.SwapEffect    = D3DSWAPEFFECT_DISCARD;
        pp.hDeviceWindow = window.handle();
        pp.Windowed      = TRUE;

        IDirect3DDevice9* device = nullptr;
        if (FAILED(d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window.handle(),
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &device)))
        {
            d3d9->Release();
            return false;
        }

        // IDirect3DDevice9 vtable indices (from IUnknown base):
        //   Reset    = 16
        //   Present  = 17
        //   EndScene = 42
        m_dx9_present_hook = safetyhook::create_inline(
            vtable_fn(device, 17),
            reinterpret_cast<void*>(&dx9_present_detour));

        m_dx9_reset_hook = safetyhook::create_inline(
            vtable_fn(device, 16),
            reinterpret_cast<void*>(&dx9_reset_detour));

        m_dx9_end_scene_hook = safetyhook::create_inline(
            vtable_fn(device, 42),
            reinterpret_cast<void*>(&dx9_end_scene_detour));

        device->Release();
        d3d9->Release();

        if (!m_dx9_present_hook || !m_dx9_reset_hook || !m_dx9_end_scene_hook)
        {
            m_dx9_present_hook   = {};
            m_dx9_reset_hook     = {};
            m_dx9_end_scene_hook = {};
            return false;
        }

        m_is_dx9_hooked = true;
        return true;
    }

    void HooksManager::unhook_dx9()
    {
        std::unique_lock lock(m_mutex);
        m_dx9_present_hook   = {};
        m_dx9_reset_hook     = {};
        m_dx9_end_scene_hook = {};
        m_is_dx9_hooked = false;
    }

    void HooksManager::set_on_dx9_present(dx9_present_callback callback)
    {
        std::unique_lock lock(m_mutex);
        m_dx9_present_cb = std::move(callback);
    }

    void HooksManager::set_on_dx9_reset(dx9_reset_callback callback)
    {
        std::unique_lock lock(m_mutex);
        m_dx9_reset_cb = std::move(callback);
    }

    void HooksManager::set_on_dx9_end_scene(dx9_end_scene_callback callback)
    {
        std::unique_lock lock(m_mutex);
        m_dx9_end_scene_cb = std::move(callback);
    }

    bool HooksManager::hook_dx11()
    {
        std::unique_lock lock(m_mutex);
        if (m_is_dx11_hooked)
            return true;

        const DummyWindow window;
        if (!window.valid())
            return false;

        const HMODULE d3d11_module = GetModuleHandle("d3d11.dll");
        if (!d3d11_module)
            return false;

        using d3d11_create_device_and_swap_chain_fn =
            HRESULT(__stdcall*)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT,
                                const D3D_FEATURE_LEVEL*, UINT, UINT,
                                const DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**,
                                ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);

        const auto create_device_and_swap_chain = reinterpret_cast<d3d11_create_device_and_swap_chain_fn>(
            GetProcAddress(d3d11_module, "D3D11CreateDeviceAndSwapChain"));
        if (!create_device_and_swap_chain)
            return false;

        DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
        swap_chain_desc.BufferDesc.Width       = 100;
        swap_chain_desc.BufferDesc.Height      = 100;
        swap_chain_desc.BufferDesc.RefreshRate = {60, 1};
        swap_chain_desc.BufferDesc.Format      = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_chain_desc.SampleDesc             = {1, 0};
        swap_chain_desc.BufferUsage            = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.BufferCount            = 1;
        swap_chain_desc.OutputWindow           = window.handle();
        swap_chain_desc.Windowed               = TRUE;
        swap_chain_desc.SwapEffect             = DXGI_SWAP_EFFECT_DISCARD;

        constexpr D3D_FEATURE_LEVEL feature_levels[] = {D3D_FEATURE_LEVEL_11_0};
        ID3D11Device*        device         = nullptr;
        ID3D11DeviceContext* device_context = nullptr;
        IDXGISwapChain*      swap_chain     = nullptr;

        if (FAILED(create_device_and_swap_chain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                                feature_levels, 1, D3D11_SDK_VERSION,
                                                &swap_chain_desc, &swap_chain,
                                                &device, nullptr, &device_context)))
            return false;

        m_dx11_present_hook = safetyhook::create_inline(
            vtable_fn(swap_chain, 8),   // IDXGISwapChain::Present
            reinterpret_cast<void*>(&dx11_present_detour));

        m_dx11_resize_buffers_hook = safetyhook::create_inline(
            vtable_fn(swap_chain, 13),  // IDXGISwapChain::ResizeBuffers
            reinterpret_cast<void*>(&dx11_resize_buffers_detour));

        swap_chain->Release();
        device_context->Release();
        device->Release();

        if (!m_dx11_present_hook || !m_dx11_resize_buffers_hook)
        {
            m_dx11_present_hook        = {};
            m_dx11_resize_buffers_hook = {};
            return false;
        }

        m_is_dx11_hooked = true;
        return true;
    }

    void HooksManager::unhook_dx11()
    {
        std::unique_lock lock(m_mutex);
        m_dx11_present_hook        = {};
        m_dx11_resize_buffers_hook = {};
        m_is_dx11_hooked = false;
    }

    bool HooksManager::hook_dx12()
    {
        std::unique_lock lock(m_mutex);
        if (m_is_dx12_hooked)
            return true;

        const DummyWindow window;
        if (!window.valid())
            return false;

        const auto fns = read_dx12_vtable_fns(window.handle());
        if (!fns)
            return false;

        m_dx12_present_hook = safetyhook::create_inline(
            fns->present,
            reinterpret_cast<void*>(&dx12_present_detour));

        m_dx12_resize_buffers_hook = safetyhook::create_inline(
            fns->resize_buffers,
            reinterpret_cast<void*>(&dx12_resize_buffers_detour));

        m_dx12_execute_command_lists_hook = safetyhook::create_inline(
            fns->execute_command_lists,
            reinterpret_cast<void*>(&dx12_execute_command_lists_detour));

        if (!m_dx12_present_hook || !m_dx12_resize_buffers_hook || !m_dx12_execute_command_lists_hook)
        {
            m_dx12_present_hook               = {};
            m_dx12_resize_buffers_hook        = {};
            m_dx12_execute_command_lists_hook = {};
            return false;
        }

        m_is_dx12_hooked = true;
        return true;
    }

    void HooksManager::unhook_dx12()
    {
        std::unique_lock lock(m_mutex);
        m_dx12_present_hook               = {};
        m_dx12_resize_buffers_hook        = {};
        m_dx12_execute_command_lists_hook = {};
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

        m_hooked_hwnd        = hwnd;
        m_original_wndproc   = prev;
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

    HRESULT __stdcall HooksManager::dx9_present_detour(IDirect3DDevice9* p_device, const RECT* p_source_rect,
                                                       const RECT* p_dest_rect, HWND h_dest_window_override,
                                                       const RGNDATA* p_dirty_region)
    {
        auto& mgr = get();
        dx9_present_callback cb;
        {
            std::shared_lock lock(mgr.m_mutex);
            cb = mgr.m_dx9_present_cb;
        }
        if (cb)
            cb(p_device, p_source_rect, p_dest_rect, h_dest_window_override, p_dirty_region);
        return mgr.m_dx9_present_hook.call<HRESULT>(p_device, p_source_rect, p_dest_rect,
                                                    h_dest_window_override, p_dirty_region);
    }

    HRESULT __stdcall HooksManager::dx9_reset_detour(IDirect3DDevice9* p_device,
                                                     D3DPRESENT_PARAMETERS* p_presentation_parameters)
    {
        auto& mgr = get();
        dx9_reset_callback cb;
        {
            std::shared_lock lock(mgr.m_mutex);
            cb = mgr.m_dx9_reset_cb;
        }
        if (cb)
            cb(p_device, p_presentation_parameters);
        return mgr.m_dx9_reset_hook.call<HRESULT>(p_device, p_presentation_parameters);
    }

    HRESULT __stdcall HooksManager::dx9_end_scene_detour(IDirect3DDevice9* p_device)
    {
        auto& mgr = get();
        dx9_end_scene_callback cb;
        {
            std::shared_lock lock(mgr.m_mutex);
            cb = mgr.m_dx9_end_scene_cb;
        }
        if (cb)
            cb(p_device);
        return mgr.m_dx9_end_scene_hook.call<HRESULT>(p_device);
    }

    HRESULT __stdcall HooksManager::dx11_present_detour(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags)
    {
        auto& mgr = get();
        present_callback cb;
        {
            std::shared_lock lock(mgr.m_mutex);
            cb = mgr.m_present_cb;
        }
        if (cb)
            cb(p_swap_chain, sync_interval, flags);
        return mgr.m_dx11_present_hook.call<HRESULT>(p_swap_chain, sync_interval, flags);
    }

    HRESULT __stdcall HooksManager::dx11_resize_buffers_detour(IDXGISwapChain* p_swap_chain, UINT buffer_count,
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
        return mgr.m_dx11_resize_buffers_hook.call<HRESULT>(p_swap_chain, buffer_count, width, height,
                                                            new_format, swap_chain_flags);
    }

    HRESULT __stdcall HooksManager::dx12_present_detour(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags)
    {
        auto& mgr = get();
        present_callback cb;
        {
            std::shared_lock lock(mgr.m_mutex);
            cb = mgr.m_present_cb;
        }
        if (cb)
            cb(p_swap_chain, sync_interval, flags);
        return mgr.m_dx12_present_hook.call<HRESULT>(p_swap_chain, sync_interval, flags);
    }

    HRESULT __stdcall HooksManager::dx12_resize_buffers_detour(IDXGISwapChain* p_swap_chain, UINT buffer_count,
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
        return mgr.m_dx12_resize_buffers_hook.call<HRESULT>(p_swap_chain, buffer_count, width, height,
                                                            new_format, swap_chain_flags);
    }

    void __stdcall HooksManager::dx12_execute_command_lists_detour(ID3D12CommandQueue* p_command_queue,
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
        mgr.m_dx12_execute_command_lists_hook.call<void>(p_command_queue, num_command_lists, pp_command_lists);
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
