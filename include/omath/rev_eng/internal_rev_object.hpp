//
// Created by Vlad on 8/8/2025.
//

#pragma once
#include <cstddef>
#include <cstdint>

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

        template<std::size_t id, class ReturnType>
        ReturnType call_virtual_method(auto... arg_list)
        {
#ifdef _MSC_VER
            using VirtualMethodType = ReturnType(__thiscall*)(void*, decltype(arg_list)...);
#else
            using VirtualMethodType = ReturnType (*)(void*, decltype(arg_list)...);
#endif
            return (*reinterpret_cast<VirtualMethodType**>(this))[id](this, arg_list...);
        }
        template<std::size_t id, class ReturnType>
        ReturnType call_virtual_method(auto... arg_list) const
        {
#ifdef _MSC_VER
            using VirtualMethodType = ReturnType(__thiscall*)(void*, decltype(arg_list)...);
#else
            using VirtualMethodType = ReturnType (*)(void*, decltype(arg_list)...);
#endif
            auto* this_ptr = const_cast<void*>(static_cast<const void*>(this));
            auto** vtable_ptr = reinterpret_cast<VirtualMethodType**>(this_ptr);
            // NOLINTNEXTLINE(clang-analyzer-core.uninitialized.Assign)
            auto* vtable = *vtable_ptr;
            return vtable[id](this_ptr, arg_list...);
        }
    };
} // namespace omath::rev_eng
