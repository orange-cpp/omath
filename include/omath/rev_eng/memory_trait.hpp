//
// Created by Vlad on 8/14/2026.
//

#pragma once
#include <cstddef>
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

    [[nodiscard("You must use shifted address")]]
    constexpr std::uintptr_t shift_address(const std::uintptr_t address, const std::ptrdiff_t offset) noexcept
    {
        return address + static_cast<std::uintptr_t>(offset);
    }

    // Reads a pointer of the target, which is not necessarily as wide as ours
    template<MemoryReadTrait MemoryTrait>
    [[nodiscard("You must use read pointer")]]
    std::uintptr_t read_pointer(const std::uintptr_t address, const std::size_t pointer_size)
    {
        if (pointer_size == sizeof(std::uint32_t))
            return MemoryTrait::template read_memory<std::uint32_t>(address);

        return MemoryTrait::template read_memory<std::uintptr_t>(address);
    }
} // namespace omath::rev_eng
