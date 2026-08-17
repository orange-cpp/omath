//
// Created by Vlad on 8/14/2026.
//

#pragma once
#include "omath/rev_eng/memory_trait.hpp"
#include <cstddef>
#include <cstdint>
#include <expected>
#include <utility>

namespace omath::unreal_engine
{
    enum class ObjectArrayError : std::uint8_t
    {
        // GObjects itself, or a table behind it, is null
        NULL_POINTER,
        INDEX_OUT_OF_RANGE,

        // The slot exists, but holds no object anymore
        NULL_OBJECT,
    };

    // How an object index is turned into an element of the global object array.
    enum class ObjectArrayKind : std::uint8_t
    {
        // UE 2.5 / UE 3 TArray<UObject*> and UE 4 FFixedUObjectArray: element = Objects + index * item_stride
        FLAT,

        // UE 4.11+ / UE 5 FChunkedFixedUObjectArray:
        // element = Objects[index / elements_per_chunk] + index % elements_per_chunk * item_stride
        CHUNKED,
    };

    // Every field here is engine AND build specific. Presets below hold the values that are typical for their engine
    // generation, but games do move members around, so verify them with a dumper before shipping.
    struct ObjectArrayLayout final
    {
        ObjectArrayKind kind{ObjectArrayKind::CHUNKED};

        // GObjects -> TArray::Data / FUObjectArray::ObjObjects::Objects
        std::ptrdiff_t objects_offset{0x10};

        // GObjects -> TArray::ArrayNum / FUObjectArray::ObjObjects::NumElements
        std::ptrdiff_t count_offset{0x24};

        // CHUNKED only
        std::size_t elements_per_chunk{64 * 1024};

        // CHUNKED only. True when objects_offset holds a pointer to the chunk table (UE 4.11+, UE 5), false when the
        // chunk table is an array inlined into the object array itself (UE 4.0 - 4.10).
        bool indirect_chunk_table{true};

        // Size of one element: sizeof(FUObjectItem) on UE 4 and UE 5, a bare pointer on UE 2.5 and UE 3
        std::size_t item_stride{0x18};

        // Element -> UObject*, FUObjectItem keeps it first
        std::ptrdiff_t item_object_offset{0x0};

        // Size of a pointer in the TARGET process - not sizeof(std::uintptr_t) of whoever compiles this, which would
        // silently flip if the tool itself is ever built as x86. Defaults to 8, the ue2_5()/ue3() preset below sets it
        // to 4 together with item_stride for its x86 target.
        std::size_t pointer_size{sizeof(std::uint64_t)};

        // UE 2.5 and UE 3 (x86): UObject::GObjObjects is a plain TArray<UObject*>
        [[nodiscard("You must use object array layout")]]
        static constexpr ObjectArrayLayout ue2_5() noexcept
        {
            return {
                    .kind = ObjectArrayKind::FLAT,
                    .objects_offset = 0x0,
                    .count_offset = 0x4,
                    .item_stride = sizeof(std::uint32_t),
                    .pointer_size = sizeof(std::uint32_t),
            };
        }

        [[nodiscard("You must use object array layout")]]
        static constexpr ObjectArrayLayout ue3() noexcept
        {
            return ue2_5();
        }

        // UE 4 built with a FFixedUObjectArray (x64): FUObjectItem* Objects, MaxElements, NumElements
        [[nodiscard("You must use object array layout")]]
        static constexpr ObjectArrayLayout ue4_fixed() noexcept
        {
            return {
                    .kind = ObjectArrayKind::FLAT,
                    .objects_offset = 0x10,
                    .count_offset = 0x1C,
            };
        }

        // UE 4.11+ (x64): FChunkedFixedUObjectArray at FUObjectArray + 0x10, sizeof(FUObjectItem) == 0x18
        [[nodiscard("You must use object array layout")]]
        static constexpr ObjectArrayLayout ue4() noexcept
        {
            return {};
        }

        // UE 5 keeps the chunked array of UE 4.11
        [[nodiscard("You must use object array layout")]]
        static constexpr ObjectArrayLayout ue5() noexcept
        {
            return ue4();
        }
    };

    // Amount of slots the array holds, freed slots included. g_objects is the address of UObject::GObjObjects on
    // UE 2.5 and UE 3, and the address of GUObjectArray on UE 4 and UE 5.
    template<rev_eng::MemoryReadTrait MemoryTrait = rev_eng::InternalMemoryTrait>
    [[nodiscard("You must use object count")]]
    std::expected<std::uint32_t, ObjectArrayError> get_object_count(const std::uintptr_t g_objects,
                                                                    const ObjectArrayLayout& layout)
    {
        if (!g_objects)
            return std::unexpected(ObjectArrayError::NULL_POINTER);

        return MemoryTrait::template read_memory<std::uint32_t>(rev_eng::shift_address(g_objects, layout.count_offset));
    }

    // Address of the UObject stored in the given slot. Slots of destroyed objects report NULL_OBJECT, so a loop over
    // the whole array has to skip them.
    template<rev_eng::MemoryReadTrait MemoryTrait = rev_eng::InternalMemoryTrait>
    [[nodiscard("You must use object address")]]
    std::expected<std::uintptr_t, ObjectArrayError>
    get_object_by_index(const std::uintptr_t g_objects, const std::uint32_t index, const ObjectArrayLayout& layout)
    {
        const auto count = get_object_count<MemoryTrait>(g_objects, layout);

        if (!count.has_value())
            return std::unexpected(count.error());

        if (index >= *count)
            return std::unexpected(ObjectArrayError::INDEX_OUT_OF_RANGE);

        const auto objects_address = rev_eng::shift_address(g_objects, layout.objects_offset);
        std::uintptr_t element{};

        if (layout.kind == ObjectArrayKind::FLAT)
        {
            const auto objects = rev_eng::read_pointer<MemoryTrait>(objects_address, layout.pointer_size);

            if (!objects)
                return std::unexpected(ObjectArrayError::NULL_POINTER);

            element = objects + index * layout.item_stride;
        }
        else if (layout.kind == ObjectArrayKind::CHUNKED)
        {
            const auto chunks = layout.indirect_chunk_table
                                        ? rev_eng::read_pointer<MemoryTrait>(objects_address, layout.pointer_size)
                                        : objects_address;

            if (!chunks)
                return std::unexpected(ObjectArrayError::NULL_POINTER);

            const auto chunk = rev_eng::read_pointer<MemoryTrait>(
                    chunks + index / layout.elements_per_chunk * layout.pointer_size, layout.pointer_size);

            if (!chunk)
                return std::unexpected(ObjectArrayError::NULL_POINTER);

            element = chunk + index % layout.elements_per_chunk * layout.item_stride;
        }
        else
        {
            std::unreachable();
        }

        const auto object = rev_eng::read_pointer<MemoryTrait>(
                rev_eng::shift_address(element, layout.item_object_offset), layout.pointer_size);

        if (!object)
            return std::unexpected(ObjectArrayError::NULL_OBJECT);

        return object;
    }
} // namespace omath::unreal_engine
