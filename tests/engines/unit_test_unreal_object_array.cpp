//
// Created by Vlad on 8/14/2026.
//
#include <cstring>
#include <gtest/gtest.h>
#include <memory>
#include <omath/engines/unreal_engine/actor_name.hpp>
#include <omath/engines/unreal_engine/object_array.hpp>
#include <string_view>
#include <vector>

using omath::unreal_engine::get_object_by_index;
using omath::unreal_engine::get_object_count;
using omath::unreal_engine::ObjectArrayError;
using omath::unreal_engine::ObjectArrayLayout;

namespace
{
    // Owns a couple of zeroed blocks and hands out their real addresses, so InternalMemoryTrait can walk them
    class FakeGame final
    {
    public:
        [[nodiscard]]
        std::uintptr_t allocate(const std::size_t size)
        {
            m_blocks.push_back(std::make_unique<std::byte[]>(size));

            return reinterpret_cast<std::uintptr_t>(m_blocks.back().get());
        }

        template<class Type>
        void write(const std::uintptr_t address, const Type& value) const
        {
            std::memcpy(reinterpret_cast<void*>(address), &value, sizeof(Type));
        }

        void write_ansi(const std::uintptr_t address, const std::string_view text) const
        {
            std::memcpy(reinterpret_cast<void*>(address), text.data(), text.size());
        }

    private:
        std::vector<std::unique_ptr<std::byte[]>> m_blocks;
    };

    constexpr std::size_t k_item_stride = 0x18;
    constexpr std::uint32_t k_elements_per_chunk = 64 * 1024;
} // namespace

TEST(unit_test_unreal_object_array, ue5_chunked_array)
{
    FakeGame game;

    const auto actor = game.allocate(0x30);

    // Second chunk, third item inside it
    constexpr std::uint32_t index = k_elements_per_chunk + 2;

    const auto chunk = game.allocate(0x100);
    game.write<std::uintptr_t>(chunk + 2 * k_item_stride, actor);

    const auto chunks = game.allocate(0x40);
    game.write<std::uintptr_t>(chunks + sizeof(std::uintptr_t), chunk);

    const auto g_objects = game.allocate(0x40);
    game.write<std::uintptr_t>(g_objects + 0x10, chunks);
    game.write<std::uint32_t>(g_objects + 0x24, index + 1);

    // FakeGame hands out real addresses of this very process via InternalMemoryTrait, so the pointer width used for
    // the chunk table stride above has to match this test binary's own build, not ue5()'s fixed 8 bytes
    auto layout = ObjectArrayLayout::ue5();
    layout.pointer_size = sizeof(std::uintptr_t);

    const auto count = get_object_count(g_objects, layout);

    ASSERT_TRUE(count.has_value());
    EXPECT_EQ(*count, index + 1);

    const auto object = get_object_by_index(g_objects, index, layout);

    ASSERT_TRUE(object.has_value());
    EXPECT_EQ(*object, actor);
}

TEST(unit_test_unreal_object_array, ue4_chunked_array_first_chunk)
{
    FakeGame game;

    const auto actor = game.allocate(0x30);

    const auto chunk = game.allocate(0x100);
    game.write<std::uintptr_t>(chunk, actor);

    const auto chunks = game.allocate(0x40);
    game.write<std::uintptr_t>(chunks, chunk);

    const auto g_objects = game.allocate(0x40);
    game.write<std::uintptr_t>(g_objects + 0x10, chunks);
    game.write<std::uint32_t>(g_objects + 0x24, 1);

    const auto object = get_object_by_index(g_objects, 0, ObjectArrayLayout::ue4());

    ASSERT_TRUE(object.has_value());
    EXPECT_EQ(*object, actor);
}

TEST(unit_test_unreal_object_array, ue4_inlined_chunk_table)
{
    FakeGame game;

    const auto actor = game.allocate(0x30);

    // UE 4.0 - 4.10 keeps an array of UObject* per chunk and the chunk table inside the array object itself
    const auto chunk = game.allocate(0x100);
    game.write<std::uintptr_t>(chunk + sizeof(std::uintptr_t), actor);

    const auto g_objects = game.allocate(0x2000);
    game.write<std::uintptr_t>(g_objects + 0x10, chunk);
    game.write<std::uint32_t>(g_objects + 0x1010, 2);

    auto layout = ObjectArrayLayout::ue4();
    layout.indirect_chunk_table = false;
    layout.elements_per_chunk = 16384;
    layout.item_stride = sizeof(std::uintptr_t);
    layout.count_offset = 0x1010;

    const auto object = get_object_by_index(g_objects, 1, layout);

    ASSERT_TRUE(object.has_value());
    EXPECT_EQ(*object, actor);
}

TEST(unit_test_unreal_object_array, ue4_fixed_array)
{
    FakeGame game;

    const auto actor = game.allocate(0x30);

    const auto objects = game.allocate(0x100);
    game.write<std::uintptr_t>(objects + 3 * k_item_stride, actor);

    const auto g_objects = game.allocate(0x40);
    game.write<std::uintptr_t>(g_objects + 0x10, objects);
    game.write<std::uint32_t>(g_objects + 0x1C, 4);

    const auto object = get_object_by_index(g_objects, 3, ObjectArrayLayout::ue4_fixed());

    ASSERT_TRUE(object.has_value());
    EXPECT_EQ(*object, actor);
}

TEST(unit_test_unreal_object_array, null_g_objects_is_rejected)
{
    EXPECT_EQ(get_object_count(0, ObjectArrayLayout::ue5()).error(), ObjectArrayError::NULL_POINTER);
    EXPECT_EQ(get_object_by_index(0, 0, ObjectArrayLayout::ue5()).error(), ObjectArrayError::NULL_POINTER);
}

TEST(unit_test_unreal_object_array, index_past_the_count_is_rejected)
{
    FakeGame game;

    const auto g_objects = game.allocate(0x40);
    game.write<std::uint32_t>(g_objects + 0x24, 10);

    EXPECT_EQ(get_object_by_index(g_objects, 10, ObjectArrayLayout::ue5()).error(),
              ObjectArrayError::INDEX_OUT_OF_RANGE);
}

TEST(unit_test_unreal_object_array, missing_chunk_is_reported)
{
    FakeGame game;

    const auto chunks = game.allocate(0x40);

    const auto g_objects = game.allocate(0x40);
    game.write<std::uintptr_t>(g_objects + 0x10, chunks);
    game.write<std::uint32_t>(g_objects + 0x24, 4);

    EXPECT_EQ(get_object_by_index(g_objects, 1, ObjectArrayLayout::ue5()).error(), ObjectArrayError::NULL_POINTER);
}

TEST(unit_test_unreal_object_array, freed_slot_is_reported)
{
    FakeGame game;

    const auto chunk = game.allocate(0x100);

    const auto chunks = game.allocate(0x40);
    game.write<std::uintptr_t>(chunks, chunk);

    const auto g_objects = game.allocate(0x40);
    game.write<std::uintptr_t>(g_objects + 0x10, chunks);
    game.write<std::uint32_t>(g_objects + 0x24, 4);

    EXPECT_EQ(get_object_by_index(g_objects, 2, ObjectArrayLayout::ue5()).error(), ObjectArrayError::NULL_OBJECT);
}

TEST(unit_test_unreal_object_array, iterating_the_array_resolves_names)
{
    using omath::unreal_engine::get_actor_name;
    using omath::unreal_engine::NameLayout;

    FakeGame game;

    // One FNamePool block holding "Pawn" and "Light"
    const auto block = game.allocate(0x40);
    game.write<std::uint16_t>(block, static_cast<std::uint16_t>(4 << 6));
    game.write_ansi(block + 0x2, "Pawn");
    game.write<std::uint16_t>(block + 0x10, static_cast<std::uint16_t>(5 << 6));
    game.write_ansi(block + 0x12, "Light");

    const auto g_names = game.allocate(0x30);
    game.write<std::uintptr_t>(g_names + 0x10, block);

    const auto pawn = game.allocate(0x30);
    game.write<std::uint32_t>(pawn + 0x18, 0);
    game.write<std::uint32_t>(pawn + 0x1C, 2);

    const auto light = game.allocate(0x30);
    game.write<std::uint32_t>(light + 0x18, 0x8);

    const auto chunk = game.allocate(0x100);
    game.write<std::uintptr_t>(chunk, pawn);
    game.write<std::uintptr_t>(chunk + 2 * k_item_stride, light);

    const auto chunks = game.allocate(0x40);
    game.write<std::uintptr_t>(chunks, chunk);

    const auto g_objects = game.allocate(0x40);
    game.write<std::uintptr_t>(g_objects + 0x10, chunks);
    game.write<std::uint32_t>(g_objects + 0x24, 3);

    const auto count = get_object_count(g_objects, ObjectArrayLayout::ue5());

    ASSERT_TRUE(count.has_value());

    std::vector<std::string> names;

    for (std::uint32_t i = 0; i < *count; ++i)
    {
        const auto object = get_object_by_index(g_objects, i, ObjectArrayLayout::ue5());

        if (!object.has_value())
            continue;

        const auto name = get_actor_name(*object, g_names, NameLayout::ue5());

        ASSERT_TRUE(name.has_value());
        names.push_back(*name);
    }

    ASSERT_EQ(names.size(), 2u);
    EXPECT_EQ(names[0], "Pawn_1");
    EXPECT_EQ(names[1], "Light");
}

namespace
{
    // Simulates a 32 bit game read from the outside: an address is just an offset inside this buffer
    std::vector<std::byte> g_target_memory;

    struct FakeExternalMemoryTrait final
    {
        template<class Type>
        [[nodiscard]]
        static Type read_memory(const std::uintptr_t address)
        {
            Type value{};

            if (address + sizeof(Type) <= g_target_memory.size())
                std::memcpy(&value, g_target_memory.data() + address, sizeof(Type));

            return value;
        }
    };

    template<class Type>
    void write_target(const std::uintptr_t address, const Type& value)
    {
        std::memcpy(g_target_memory.data() + address, &value, sizeof(Type));
    }
} // namespace

TEST(unit_test_unreal_object_array, ue3_32_bit_target_read_externally)
{
    g_target_memory.assign(0x1000, std::byte{});

    constexpr std::uintptr_t g_objects = 0x100;
    constexpr std::uintptr_t objects = 0x200;
    constexpr std::uintptr_t actor = 0x300;

    // TArray<UObject*>: Data, ArrayNum, ArrayMax
    write_target<std::uint32_t>(g_objects, objects);
    write_target<std::uint32_t>(g_objects + 0x4, 6);
    write_target<std::uint32_t>(objects + 5 * sizeof(std::uint32_t), actor);

    auto layout = ObjectArrayLayout::ue3();
    layout.pointer_size = sizeof(std::uint32_t);
    layout.item_stride = sizeof(std::uint32_t);

    const auto count = get_object_count<FakeExternalMemoryTrait>(g_objects, layout);

    ASSERT_TRUE(count.has_value());
    EXPECT_EQ(*count, 6u);

    const auto object = get_object_by_index<FakeExternalMemoryTrait>(g_objects, 5, layout);

    ASSERT_TRUE(object.has_value());
    EXPECT_EQ(*object, actor);
}
