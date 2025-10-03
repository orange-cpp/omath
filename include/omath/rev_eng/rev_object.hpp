//
// Created by Vlad on 8/8/2025.
//

#pragma once
#include <cstddef>
#include <cstdint>

namespace omath::rev_eng
{
    class ReverseEngineeredObject
    {
    protected:
        template<class Type>
        [[nodiscard]] Type& get_by_offset(const std::ptrdiff_t offset) const
        {
            return *reinterpret_cast<Type*>(reinterpret_cast<std::uintptr_t>(this) + offset);
        }

        template<size_t id, class ReturnType>
        ReturnType call_virtual_method(auto... arg_list)
        {
            using Func = ReturnType(__thiscall*)(void*, decltype(arg_list)...);
            return (*static_cast<Func**>(this))[id](this, arg_list...);
        }
    };
} // namespace orev
