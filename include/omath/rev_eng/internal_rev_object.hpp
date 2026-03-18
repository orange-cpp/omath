//
// Created by Vlad on 8/8/2025.
//

#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>

#ifdef _WIN32
#include "omath/utility/pe_pattern_scan.hpp"
#include <windows.h>
#elif defined(__APPLE__)
#include "omath/utility/macho_pattern_scan.hpp"
#include <mach-o/dyld.h>
#else
#include "omath/utility/elf_pattern_scan.hpp"
#include <link.h>
#endif

namespace omath::rev_eng
{
    class InternalReverseEngineeredObject
    {
    protected:
        template<class Type>
        [[nodiscard]] Type& get_by_offset(const std::ptrdiff_t offset)
        {
            return *reinterpret_cast<Type*>(reinterpret_cast<std::uintptr_t>(this) + offset);
        }

        template<class Type>
        [[nodiscard]] const Type& get_by_offset(const std::ptrdiff_t offset) const
        {
            return *reinterpret_cast<Type*>(reinterpret_cast<std::uintptr_t>(this) + offset);
        }

        template<class ReturnType>
        ReturnType call_method(const void* ptr, auto... arg_list)
        {
#ifdef _MSC_VER
            using MethodType = ReturnType(__thiscall*)(void*, decltype(arg_list)...);
#else
            using MethodType = ReturnType (*)(void*, decltype(arg_list)...);
#endif
            return reinterpret_cast<MethodType>(const_cast<void*>(ptr))(this, arg_list...);
        }
        template<class ReturnType>
        ReturnType call_method(const void* ptr, auto... arg_list) const
        {
#ifdef _MSC_VER
            using MethodType = ReturnType(__thiscall*)(const void*, decltype(arg_list)...);
#else
            using MethodType = ReturnType (*)(const void*, decltype(arg_list)...);
#endif
            return reinterpret_cast<MethodType>(const_cast<void*>(ptr))(this, arg_list...);
        }

        template<auto module_name, auto pattern, class ReturnType>
        ReturnType call_method(auto... arg_list)
        {
            static const auto* address = resolve_pattern(module_name, pattern);
            return call_method<ReturnType>(address, arg_list...);
        }

        template<auto module_name, auto pattern, class ReturnType>
        ReturnType call_method(auto... arg_list) const
        {
            static const auto* address = resolve_pattern(module_name, pattern);
            return call_method<ReturnType>(address, arg_list...);
        }

        template<std::size_t id, class ReturnType>
        ReturnType call_virtual_method(auto... arg_list)
        {
            const auto vtable = *reinterpret_cast<void***>(this);
            return call_method<ReturnType>(vtable[id], arg_list...);
        }
        template<std::size_t id, class ReturnType>
        ReturnType call_virtual_method(auto... arg_list) const
        {
            const auto vtable = *reinterpret_cast<void* const* const*>(this);
            return call_method<ReturnType>(vtable[id], arg_list...);
        }

    private:
        static const void* resolve_pattern(const std::string_view module_name, const std::string_view pattern)
        {
            const auto* base = get_module_base(module_name);
            assert(base && "Failed to find module");

#ifdef _WIN32
            auto result = PePatternScanner::scan_for_pattern_in_loaded_module(base, pattern);
#elif defined(__APPLE__)
            auto result = MachOPatternScanner::scan_for_pattern_in_loaded_module(base, pattern);
#else
            auto result = ElfPatternScanner::scan_for_pattern_in_loaded_module(base, pattern);
#endif
            assert(result.has_value() && "Pattern scan failed");
            return reinterpret_cast<const void*>(*result);
        }

        static const void* get_module_base(const std::string_view module_name)
        {
#ifdef _WIN32
            return static_cast<const void*>(GetModuleHandleA(module_name.data()));
#elif defined(__APPLE__)
            // On macOS, iterate loaded images to find the module by name
            const auto count = _dyld_image_count();
            for (std::uint32_t i = 0; i < count; ++i)
            {
                const auto* name = _dyld_get_image_name(i);
                if (name && std::string_view{name}.find(module_name) != std::string_view::npos)
                    return static_cast<const void*>(_dyld_get_image_header(i));
            }
            return nullptr;
#else
            // On Linux, use dl_iterate_phdr to find loaded module by name
            struct CallbackData
            {
                std::string_view name;
                const void* base;
            } cb_data{module_name, nullptr};

            dl_iterate_phdr(
                    [](dl_phdr_info* info, std::size_t, void* data) -> int
                    {
                        auto* cb = static_cast<CallbackData*>(data);
                        if (info->dlpi_name
                            && std::string_view{info->dlpi_name}.find(cb->name) != std::string_view::npos)
                        {
                            cb->base = reinterpret_cast<const void*>(info->dlpi_addr);
                            return 1;
                        }
                        return 0;
                    },
                    &cb_data);
            return cb_data.base;
#endif
        }
    };
} // namespace omath::rev_eng
