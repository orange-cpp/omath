#pragma once

#ifdef OMATH_ENABLE_HOOKING
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <shared_mutex>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <d3d12.h>
#include <d3d9.h>
#include <dxgi.h>
#include <safetyhook.hpp>

namespace omath::hooks
{
    class HooksManager final
    {
        HooksManager() = default;

    public:
        // IDXGISwapChain callbacks — shared between DX11 and DX12 (same interface, same signature).
        using present_callback = std::function<void(IDXGISwapChain*, UINT, UINT)>;
        using resize_buffers_callback = std::function<void(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)>;
        using execute_command_lists_callback =
                std::function<void(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*)>;

        // IDirect3DDevice9 callbacks — DX9 only.
        using dx9_present_callback =
                std::function<void(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*)>;
        using dx9_reset_callback = std::function<void(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*)>;
        using dx9_end_scene_callback = std::function<void(IDirect3DDevice9*)>;

        // OpenGL callbacks. Fires before the hooked buffer swap function calls the original.
        using opengl_swap_buffers_callback = std::function<void(HDC)>;

        // Return nullopt to pass the message to the original WndProc; return a value to intercept it.
        using wnd_proc_callback = std::function<std::optional<LRESULT>(HWND, UINT, WPARAM, LPARAM)>;

        template<typename Callback>
        using callback_ptr = std::shared_ptr<const Callback>;

        [[nodiscard]] static HooksManager& get();
        HooksManager(const HooksManager&) = delete;
        HooksManager& operator=(const HooksManager&) = delete;
        ~HooksManager();

        [[nodiscard]] bool hook_dx9();
        void unhook_dx9();
        void set_on_dx9_present(dx9_present_callback callback);
        void set_on_dx9_reset(dx9_reset_callback callback);
        void set_on_dx9_end_scene(dx9_end_scene_callback callback);

        [[nodiscard]] bool hook_dx11();
        void unhook_dx11();

        [[nodiscard]] bool hook_dx12();
        void unhook_dx12();

        [[nodiscard]] bool hook_opengl();
        void unhook_opengl();
        void set_on_opengl_swap_buffers(opengl_swap_buffers_callback callback);

        // Present and ResizeBuffers callbacks fire for whichever of DX11/DX12 is hooked.
        void set_on_present(present_callback callback);
        void set_on_resize_buffers(resize_buffers_callback callback);
        void set_on_execute_command_lists(execute_command_lists_callback callback);

        [[nodiscard]] bool hook_wnd_proc(HWND hwnd);
        void unhook_wnd_proc();
        void set_on_wnd_proc(wnd_proc_callback callback);

    private:
        [[nodiscard]]
        static HRESULT __stdcall dx9_present_detour(IDirect3DDevice9* p_device, const RECT* p_source_rect,
                                                    const RECT* p_dest_rect, HWND h_dest_window_override,
                                                    const RGNDATA* p_dirty_region);
        [[nodiscard]]
        static HRESULT __stdcall dx9_reset_detour(IDirect3DDevice9* p_device,
                                                  D3DPRESENT_PARAMETERS* p_presentation_parameters);
        [[nodiscard]]
        static HRESULT __stdcall dx9_end_scene_detour(IDirect3DDevice9* p_device);

        [[nodiscard]]
        static HRESULT __stdcall dx11_present_detour(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags);
        [[nodiscard]]
        static HRESULT __stdcall dx11_resize_buffers_detour(IDXGISwapChain* p_swap_chain, UINT buffer_count, UINT width,
                                                            UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags);
        [[nodiscard]]
        static HRESULT __stdcall dx12_present_detour(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags);
        [[nodiscard]]
        static HRESULT __stdcall dx12_resize_buffers_detour(IDXGISwapChain* p_swap_chain, UINT buffer_count, UINT width,
                                                            UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags);
        static void __stdcall dx12_execute_command_lists_detour(ID3D12CommandQueue* p_command_queue,
                                                                UINT num_command_lists,
                                                                ID3D12CommandList* const* pp_command_lists);

        [[nodiscard]]
        static BOOL __stdcall opengl_wgl_swap_buffers_detour(HDC hdc);
        [[nodiscard]]
        static BOOL __stdcall opengl_swap_buffers_detour(HDC hdc);

        [[nodiscard]]
        static LRESULT __stdcall wnd_proc_detour(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param);

        mutable std::shared_mutex m_hook_state_mutex;

        mutable std::shared_mutex m_dx9_present_mutex;
        mutable std::shared_mutex m_dx9_reset_mutex;
        mutable std::shared_mutex m_dx9_end_scene_mutex;

        mutable std::shared_mutex m_present_mutex;
        mutable std::shared_mutex m_resize_buffers_mutex;
        mutable std::shared_mutex m_execute_command_lists_mutex;

        mutable std::shared_mutex m_opengl_swap_buffers_mutex;
        mutable std::shared_mutex m_wnd_proc_mutex;

        bool m_is_dx9_hooked = false;
        bool m_is_dx11_hooked = false;
        bool m_is_dx12_hooked = false;
        bool m_is_opengl_hooked = false;
        bool m_is_wnd_proc_hooked = false;

        HWND m_hooked_hwnd = nullptr;
        WNDPROC m_original_wndproc = nullptr;

        safetyhook::InlineHook m_dx9_present_hook;
        safetyhook::InlineHook m_dx9_reset_hook;
        safetyhook::InlineHook m_dx9_end_scene_hook;

        safetyhook::InlineHook m_dx11_present_hook;
        safetyhook::InlineHook m_dx11_resize_buffers_hook;

        safetyhook::InlineHook m_dx12_present_hook;
        safetyhook::InlineHook m_dx12_resize_buffers_hook;
        safetyhook::InlineHook m_dx12_execute_command_lists_hook;

        safetyhook::InlineHook m_opengl_wgl_swap_buffers_hook;
        safetyhook::InlineHook m_opengl_swap_buffers_hook;

        callback_ptr<dx9_present_callback> m_dx9_present_cb;
        callback_ptr<dx9_reset_callback> m_dx9_reset_cb;
        callback_ptr<dx9_end_scene_callback> m_dx9_end_scene_cb;

        callback_ptr<present_callback> m_present_cb;
        callback_ptr<resize_buffers_callback> m_resize_buffers_cb;
        callback_ptr<execute_command_lists_callback> m_execute_command_lists_cb;
        callback_ptr<opengl_swap_buffers_callback> m_opengl_swap_buffers_cb;
        callback_ptr<wnd_proc_callback> m_wnd_proc_cb;
    };
} // namespace omath::hooks

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
} // namespace omath::hooks

#endif
