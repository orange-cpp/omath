//
// Created by Vlad on 8/14/2026.
//

#pragma once
#include "omath/rev_eng/memory_trait.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <utility>

namespace omath::unreal_engine
{
    enum class ActorNameError : std::uint8_t
    {
        NULL_POINTER,
        INVALID_NAME_INDEX,
        EMPTY_NAME,
    };

    // How an FName index is turned into an FNameEntry address.
    enum class NamePoolKind : std::uint8_t
    {
        // UE 2.5 / UE 3: TArray<FNameEntry*>, entry = Data[index]
        POINTER_ARRAY,

        // UE 4.0 - 4.22: TStaticIndirectArrayThreadSafeRead<FNameEntry>,
        // entry = Chunks[index / elements_per_chunk][index % elements_per_chunk]
        CHUNKED_ARRAY,

        // UE 4.23+ / UE 5: FNamePool,
        // entry = Blocks[index >> block_offset_bits] + (index & block_offset_mask) * block_entry_stride
        BLOCK_POOL,
    };

    enum class NameCharKind : std::uint8_t
    {
        ANSI,
        WIDE,

        // Bit 0 of FNameEntry::Index (UE 3, UE 4 legacy) or of FNameEntryHeader (UE 4.23+, UE 5)
        PER_ENTRY_FLAG,
    };

    // Every field here is engine AND build specific. Presets below hold the values that are typical for their engine
    // generation, but games do move members around, so verify them with a dumper before shipping.
    struct NameLayout final
    {
        // UObject -> FName (UObject::Name / UObject::NamePrivate)
        std::ptrdiff_t object_name_offset{0x18};

        // FName -> Number, relative to the FName itself. Empty when FName carries no instance number (UE 2.5).
        std::optional<std::ptrdiff_t> name_number_offset{0x4};

        NamePoolKind pool_kind{NamePoolKind::BLOCK_POOL};

        // GNames -> TArray::Data (POINTER_ARRAY), chunk table (CHUNKED_ARRAY) or block table (BLOCK_POOL)
        std::ptrdiff_t pool_data_offset{0x10};

        // Size of a pointer inside the pool tables, in the TARGET process - not sizeof(std::uintptr_t) of whoever
        // compiles this, which would silently flip if the tool itself is ever built as x86. Defaults to 8, the ue2_5()
        // and ue3() presets below set it to 4 for their x86 targets.
        std::size_t pointer_size{sizeof(std::uint64_t)};

        // CHUNKED_ARRAY only
        std::size_t elements_per_chunk{16384};

        // BLOCK_POOL only
        std::uint32_t block_offset_bits{16};
        std::uint32_t block_entry_stride{2};

        // FNameEntry -> Index (POINTER_ARRAY, CHUNKED_ARRAY) or FNameEntryHeader (BLOCK_POOL)
        std::ptrdiff_t entry_flags_offset{0x0};

        // FNameEntry -> name characters
        std::ptrdiff_t entry_text_offset{0x2};

        NameCharKind char_kind{NameCharKind::PER_ENTRY_FLAG};

        // BLOCK_POOL only: FNameEntryHeader >> shift == length. 6 by default, 1 with WITH_CASE_PRESERVING_NAME.
        std::uint32_t header_length_shift{6};

        // Read guard for null terminated names (POINTER_ARRAY, CHUNKED_ARRAY)
        std::size_t max_name_length{1024};

        // UE 2.5 (x86), derived from Killing Floor 1 build 1065. FNameEntry: Index, Flags, HashNext, Name, and that
        // build is a UNICODE one - switch char_kind to ANSI for a game that stores its names as ANSICHAR. FName is a
        // bare index here, instance numbers arrived in UE 3.
        [[nodiscard("You must use name layout")]]
        static constexpr NameLayout ue2_5() noexcept
        {
            return {
                    .object_name_offset = 0x24,
                    .name_number_offset = std::nullopt,
                    .pool_kind = NamePoolKind::POINTER_ARRAY,
                    .pool_data_offset = 0x0,
                    .pointer_size = sizeof(std::uint32_t),
                    .entry_flags_offset = 0x0,
                    .entry_text_offset = 0xC,
                    .char_kind = NameCharKind::WIDE,
            };
        }

        // UE 3 (x86). FNameEntry: 8 unknown bytes, Index, 4 unknown bytes, Name.
        [[nodiscard("You must use name layout")]]
        static constexpr NameLayout ue3() noexcept
        {
            return {
                    .object_name_offset = 0x28,
                    .pool_kind = NamePoolKind::POINTER_ARRAY,
                    .pool_data_offset = 0x0,
                    .pointer_size = sizeof(std::uint32_t),
                    .entry_flags_offset = 0x8,
                    .entry_text_offset = 0x10,
            };
        }

        // UE 4.0 - 4.22 (x64). FNameEntry: Index, padding, HashNext, Name.
        [[nodiscard("You must use name layout")]]
        static constexpr NameLayout ue4_legacy() noexcept
        {
            return {
                    .pool_kind = NamePoolKind::CHUNKED_ARRAY,
                    .pool_data_offset = 0x0,
                    .entry_flags_offset = 0x0,
                    .entry_text_offset = 0x10,
            };
        }

        // UE 4.23+ (x64, Windows). pool_data_offset is FNameEntryAllocator::Blocks, it sits behind an FRWLock, so it
        // grows to 0x40 on Linux where FRWLock wraps pthread_rwlock_t.
        [[nodiscard("You must use name layout")]]
        static constexpr NameLayout ue4() noexcept
        {
            return {};
        }

        // UE 5 keeps the FNamePool layout introduced in UE 4.23.
        [[nodiscard("You must use name layout")]]
        static constexpr NameLayout ue5() noexcept
        {
            return ue4();
        }
    };

    namespace detail
    {
        using rev_eng::read_pointer;
        using rev_eng::shift_address;

        inline void append_as_utf8(std::string& text, const char16_t unit)
        {
            if (unit < 0x80)
            {
                text.push_back(static_cast<char>(unit));
                return;
            }
            if (unit < 0x800)
            {
                text.push_back(static_cast<char>(0xC0 | unit >> 6));
                text.push_back(static_cast<char>(0x80 | (unit & 0x3F)));
                return;
            }
            text.push_back(static_cast<char>(0xE0 | unit >> 12));
            text.push_back(static_cast<char>(0x80 | (unit >> 6 & 0x3F)));
            text.push_back(static_cast<char>(0x80 | (unit & 0x3F)));
        }

        template<rev_eng::MemoryReadTrait MemoryTrait>
        [[nodiscard("You must use name entry address")]]
        std::uintptr_t name_entry_address(const std::uintptr_t g_names, const std::uint32_t name_index,
                                          const NameLayout& layout)
        {
            const auto pointer_size = layout.pointer_size;
            const auto pool_data = shift_address(g_names, layout.pool_data_offset);

            if (layout.pool_kind == NamePoolKind::POINTER_ARRAY)
            {
                const auto entries = read_pointer<MemoryTrait>(pool_data, pointer_size);

                if (!entries)
                    return 0;

                return read_pointer<MemoryTrait>(entries + name_index * pointer_size, pointer_size);
            }

            if (layout.pool_kind == NamePoolKind::CHUNKED_ARRAY)
            {
                const auto chunk = read_pointer<MemoryTrait>(
                        pool_data + name_index / layout.elements_per_chunk * pointer_size, pointer_size);

                if (!chunk)
                    return 0;

                return read_pointer<MemoryTrait>(chunk + name_index % layout.elements_per_chunk * pointer_size,
                                                 pointer_size);
            }

            if (layout.pool_kind == NamePoolKind::BLOCK_POOL)
            {
                const auto block = read_pointer<MemoryTrait>(
                        pool_data + (name_index >> layout.block_offset_bits) * pointer_size, pointer_size);

                if (!block)
                    return 0;

                const auto block_offset_mask = (std::uint32_t{1} << layout.block_offset_bits) - 1;

                return block + (name_index & block_offset_mask) * layout.block_entry_stride;
            }

            std::unreachable();
        }

        template<rev_eng::MemoryReadTrait MemoryTrait>
        [[nodiscard("You must use name text")]]
        std::string name_entry_text(const std::uintptr_t name_entry, const NameLayout& layout)
        {
            auto is_wide = layout.char_kind == NameCharKind::WIDE;
            auto length = layout.max_name_length;

            if (layout.char_kind == NameCharKind::PER_ENTRY_FLAG || layout.pool_kind == NamePoolKind::BLOCK_POOL)
            {
                const auto flags = MemoryTrait::template read_memory<std::uint16_t>(
                        shift_address(name_entry, layout.entry_flags_offset));

                if (layout.char_kind == NameCharKind::PER_ENTRY_FLAG)
                    is_wide = (flags & 1) != 0;

                // Only FNamePool stores the length inside the entry, older generations are null terminated
                if (layout.pool_kind == NamePoolKind::BLOCK_POOL)
                    length = std::min(static_cast<std::size_t>(flags >> layout.header_length_shift),
                                      layout.max_name_length);
            }

            const auto text_address = shift_address(name_entry, layout.entry_text_offset);
            std::string text;

            for (std::size_t i = 0; i < length; ++i)
            {
                const auto unit =
                        is_wide ? MemoryTrait::template read_memory<char16_t>(text_address + i * sizeof(char16_t))
                                : static_cast<char16_t>(
                                          MemoryTrait::template read_memory<unsigned char>(text_address + i));

                if (unit == u'\0')
                    break;

                append_as_utf8(text, unit);
            }

            return text;
        }
    } // namespace detail

    // Resolves the FName of an actor (or of any other UObject) through GNames, the way UObject::GetName() does.
    // actor and g_names are addresses inside the address space MemoryTrait reads from, g_names must point at the name
    // container itself - dereference it yourself when your signature resolves to a pointer to it.
    template<rev_eng::MemoryReadTrait MemoryTrait = rev_eng::InternalMemoryTrait>
    [[nodiscard("You must use actor name")]]
    std::expected<std::string, ActorNameError> get_actor_name(const std::uintptr_t actor, const std::uintptr_t g_names,
                                                              const NameLayout& layout)
    {
        if (!actor || !g_names)
            return std::unexpected(ActorNameError::NULL_POINTER);

        const auto name_address = detail::shift_address(actor, layout.object_name_offset);
        const auto name_index = MemoryTrait::template read_memory<std::uint32_t>(name_address);
        const auto name_entry = detail::name_entry_address<MemoryTrait>(g_names, name_index, layout);

        if (!name_entry)
            return std::unexpected(ActorNameError::INVALID_NAME_INDEX);

        auto name = detail::name_entry_text<MemoryTrait>(name_entry, layout);

        if (name.empty())
            return std::unexpected(ActorNameError::EMPTY_NAME);

        if (!layout.name_number_offset.has_value())
            return name;

        const auto number = MemoryTrait::template read_memory<std::uint32_t>(
                detail::shift_address(name_address, *layout.name_number_offset));

        // UE prints an instance number as a "_N" suffix, where N is the stored number decremented by one
        if (number != 0)
            name += '_' + std::to_string(number - 1);

        return name;
    }
} // namespace omath::unreal_engine
