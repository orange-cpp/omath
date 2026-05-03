#pragma once

#ifdef OMATH_ENABLE_HOOKING
#include <functional>
#include <optional>
#include <shared_mutex>
#include <cstdint>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <dxgi.h>
#include <d3d12.h>
#include <safetyhook.hpp>

namespace omath::hooks
{
    class HooksManager final
    {
        HooksManager() = default;
    public:
        // IDXGISwapChain callbacks are shared between DX11 and DX12 — same interface, same signature.
        using present_callback               = std::function<void(IDXGISwapChain*, UINT, UINT)>;
        using resize_buffers_callback        = std::function<void(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)>;
        using execute_command_lists_callback = std::function<void(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*)>;
        // Return nullopt to pass the message to the original WndProc; return a value to intercept it.
        using wnd_proc_callback              = std::function<std::optional<LRESULT>(HWND, UINT, WPARAM, LPARAM)>;

        [[nodiscard]] static HooksManager& get();
        HooksManager(const HooksManager&)            = delete;
        HooksManager& operator=(const HooksManager&) = delete;
        ~HooksManager();

        [[nodiscard]] bool hook_dx11();
        void unhook_dx11();

        [[nodiscard]] bool hook_dx12();
        void unhook_dx12();

        // Present and ResizeBuffers callbacks fire for whichever API is hooked.
        void set_on_present(present_callback callback);
        void set_on_resize_buffers(resize_buffers_callback callback);

        void set_on_execute_command_lists(execute_command_lists_callback callback);

        [[nodiscard]] bool hook_wnd_proc(HWND hwnd);
        void unhook_wnd_proc();
        void set_on_wnd_proc(wnd_proc_callback callback);

    private:
        static HRESULT __stdcall dx11_present_detour(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags);
        static HRESULT __stdcall dx11_resize_buffers_detour(IDXGISwapChain* p_swap_chain, UINT buffer_count,
                                                            UINT width, UINT height, DXGI_FORMAT new_format,
                                                            UINT swap_chain_flags);

        static HRESULT __stdcall dx12_present_detour(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags);
        static HRESULT __stdcall dx12_resize_buffers_detour(IDXGISwapChain* p_swap_chain, UINT buffer_count,
                                                            UINT width, UINT height, DXGI_FORMAT new_format,
                                                            UINT swap_chain_flags);
        static void __stdcall dx12_execute_command_lists_detour(ID3D12CommandQueue* p_command_queue,
                                                                UINT num_command_lists,
                                                                ID3D12CommandList* const* pp_command_lists);

        static LRESULT __stdcall wnd_proc_detour(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param);

        mutable std::shared_mutex m_mutex;

        bool m_is_dx11_hooked     = false;
        bool m_is_dx12_hooked     = false;
        bool m_is_wnd_proc_hooked = false;

        HWND    m_hooked_hwnd      = nullptr;
        WNDPROC m_original_wndproc = nullptr;

        safetyhook::InlineHook m_dx11_present_hook;
        safetyhook::InlineHook m_dx11_resize_buffers_hook;

        safetyhook::InlineHook m_dx12_present_hook;
        safetyhook::InlineHook m_dx12_resize_buffers_hook;
        safetyhook::InlineHook m_dx12_execute_command_lists_hook;

        present_callback               m_present_cb;
        resize_buffers_callback        m_resize_buffers_cb;
        execute_command_lists_callback m_execute_command_lists_cb;
        wnd_proc_callback              m_wnd_proc_cb;
    };
}

#else // !OMATH_ENABLE_HOOKING

namespace omath::hooks
{
    class HooksManager final
    {
        HooksManager() = default;
    public:
        [[nodiscard]] static HooksManager& get();
        HooksManager(const HooksManager&) = delete;
        ~HooksManager();
    };
}

#endif
