//
// Created by Vlad on 8/14/2026.
//

#pragma once
#include <cstdint>

namespace omath::rev_eng
{
    template<class Type>
    concept MemoryReadTrait =
            requires(const std::uintptr_t address) { Type::template read_memory<std::uint32_t>(address); };

    // Reads memory of the process omath is running in. Use it when your code is injected into the target process.
    // For an out of process target write your own trait around ReadProcessMemory/process_vm_readv/etc.
    struct InternalMemoryTrait final
    {
        template<class Type>
        [[nodiscard("You must use read value")]]
        static Type read_memory(const std::uintptr_t address) noexcept
        {
            return *reinterpret_cast<const Type*>(address);
        }
    };
} // namespace omath::rev_eng
