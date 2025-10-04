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
            using VirtualMethodType = ReturnType(__thiscall*)(void*, decltype(arg_list)...);
            return (*reinterpret_cast<VirtualMethodType**>(this))[id](this, arg_list...);
        }
    };
} // namespace omath::rev_eng
