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

    // Reads a pointer of the target, which is not necessarily as wide as ours. The non-4-byte case reads a fixed
    // std::uint64_t rather than the host's own std::uintptr_t - on a 32 bit host (x86, wasm32) std::uintptr_t is only
    // 4 bytes, which would otherwise silently truncate every 8 byte target pointer read to its low half. The result
    // still narrows to the host's own uintptr_t on return: a 32 bit host cannot represent a target address above 4GB
    // regardless of how correctly the bytes were read.
    template<MemoryReadTrait MemoryTrait>
    [[nodiscard("You must use read pointer")]]
    std::uintptr_t read_pointer(const std::uintptr_t address, const std::size_t pointer_size)
    {
        if (pointer_size == sizeof(std::uint32_t))
            return MemoryTrait::template read_memory<std::uint32_t>(address);

        return static_cast<std::uintptr_t>(MemoryTrait::template read_memory<std::uint64_t>(address));
    }
} // namespace omath::rev_eng
