//
// Created by Vlad on 8/14/2026.
//
#include <cstring>
#include <gtest/gtest.h>
#include <memory>
#include <omath/engines/unreal_engine/actor_name.hpp>
#include <string>
#include <string_view>
#include <vector>

using omath::unreal_engine::ActorNameError;
using omath::unreal_engine::get_actor_name;
using omath::unreal_engine::NameCharKind;
using omath::unreal_engine::NameLayout;

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

        void write_wide(const std::uintptr_t address, const std::u16string_view text) const
        {
            std::memcpy(reinterpret_cast<void*>(address), text.data(), text.size() * sizeof(char16_t));
        }

    private:
        std::vector<std::unique_ptr<std::byte[]>> m_blocks;
    };

    // FNamePool entry: FNameEntryHeader{bIsWide : 1, LowercaseProbeHash : 5, Len : 10} followed by the characters
    [[nodiscard]]
    std::uint16_t pool_entry_header(const std::size_t length, const bool is_wide)
    {
        return static_cast<std::uint16_t>(length << 6 | (is_wide ? 1 : 0));
    }
} // namespace

TEST(unit_test_unreal_actor_name, ue5_block_pool)
{
    FakeGame game;

    // Entry sits at block + 0x10, so its handle offset is 0x10 / stride(2) == 8
    const auto block = game.allocate(0x40);
    game.write<std::uint16_t>(block + 0x10, pool_entry_header(9, false));
    game.write_ansi(block + 0x12, "PlayerBox");

    const auto g_names = game.allocate(0x30);
    game.write<std::uintptr_t>(g_names + 0x10, block);

    const auto actor = game.allocate(0x30);
    game.write<std::uint32_t>(actor + 0x18, 0x8);
    game.write<std::uint32_t>(actor + 0x1C, 0);

    const auto name = get_actor_name(actor, g_names, NameLayout::ue5());

    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(*name, "PlayerBox");
}

TEST(unit_test_unreal_actor_name, ue5_block_pool_second_block_and_instance_number)
{
    FakeGame game;

    const auto block = game.allocate(0x40);
    game.write<std::uint16_t>(block, pool_entry_header(5, false));
    game.write_ansi(block + 0x2, "Actor");

    const auto g_names = game.allocate(0x30);
    game.write<std::uintptr_t>(g_names + 0x18, block);

    const auto actor = game.allocate(0x30);
    // Block index 1, offset 0 inside that block
    game.write<std::uint32_t>(actor + 0x18, 1u << 16);
    game.write<std::uint32_t>(actor + 0x1C, 3);

    const auto name = get_actor_name(actor, g_names, NameLayout::ue5());

    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(*name, "Actor_2");
}

TEST(unit_test_unreal_actor_name, ue5_block_pool_wide_name)
{
    FakeGame game;

    const auto block = game.allocate(0x40);
    game.write<std::uint16_t>(block, pool_entry_header(4, true));
    // "Caf" + U+00E9, spelled out to keep this file plain ASCII
    const std::u16string wide_text{u'C', u'a', u'f', static_cast<char16_t>(0x00E9)};
    game.write_wide(block + 0x2, wide_text);

    const auto g_names = game.allocate(0x30);
    game.write<std::uintptr_t>(g_names + 0x10, block);

    const auto actor = game.allocate(0x30);
    game.write<std::uint32_t>(actor + 0x18, 0);
    game.write<std::uint32_t>(actor + 0x1C, 0);

    const auto name = get_actor_name(actor, g_names, NameLayout::ue5());

    ASSERT_TRUE(name.has_value());
    // A wide entry is transcoded to UTF-8
    EXPECT_EQ(*name, "Caf\xC3\xA9");
}

TEST(unit_test_unreal_actor_name, ue5_case_preserving_name_pool)
{
    FakeGame game;

    // WITH_CASE_PRESERVING_NAME puts FNameEntryId in front of the header, and the header keeps no probe hash
    const auto block = game.allocate(0x40);
    game.write<std::uint16_t>(block + 0x4, static_cast<std::uint16_t>(5 << 1));
    game.write_ansi(block + 0x6, "Pawn0");

    const auto g_names = game.allocate(0x30);
    game.write<std::uintptr_t>(g_names + 0x10, block);

    const auto actor = game.allocate(0x30);
    game.write<std::uint32_t>(actor + 0x18, 0);

    auto layout = NameLayout::ue5();
    layout.entry_flags_offset = 0x4;
    layout.entry_text_offset = 0x6;
    layout.header_length_shift = 1;

    const auto name = get_actor_name(actor, g_names, layout);

    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(*name, "Pawn0");
}

TEST(unit_test_unreal_actor_name, ue4_legacy_chunked_array)
{
    FakeGame game;

    constexpr std::uint32_t name_index = 16384 + 2;

    const auto entry = game.allocate(0x40);
    game.write<std::int32_t>(entry, static_cast<std::int32_t>(name_index) << 1);
    game.write_ansi(entry + 0x10, "MyPawn");

    const auto chunk = game.allocate(0x40);
    game.write<std::uintptr_t>(chunk + 2 * sizeof(std::uintptr_t), entry);

    // Chunk table lives right at GNames
    const auto g_names = game.allocate(0x40);
    game.write<std::uintptr_t>(g_names + sizeof(std::uintptr_t), chunk);

    const auto actor = game.allocate(0x30);
    game.write<std::uint32_t>(actor + 0x18, name_index);
    game.write<std::uint32_t>(actor + 0x1C, 1);

    const auto name = get_actor_name(actor, g_names, NameLayout::ue4_legacy());

    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(*name, "MyPawn_0");
}

TEST(unit_test_unreal_actor_name, ue4_legacy_wide_entry)
{
    FakeGame game;

    const auto entry = game.allocate(0x40);
    // Bit 0 of FNameEntry::Index marks a wide entry
    game.write<std::int32_t>(entry, 1);
    game.write_wide(entry + 0x10, u"Wide");

    const auto chunk = game.allocate(0x40);
    game.write<std::uintptr_t>(chunk, entry);

    const auto g_names = game.allocate(0x40);
    game.write<std::uintptr_t>(g_names, chunk);

    const auto actor = game.allocate(0x30);
    game.write<std::uint32_t>(actor + 0x18, 0);

    const auto name = get_actor_name(actor, g_names, NameLayout::ue4_legacy());

    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(*name, "Wide");
}

TEST(unit_test_unreal_actor_name, ue3_pointer_array)
{
    FakeGame game;

    const auto entry = game.allocate(0x40);
    game.write<std::int32_t>(entry + 0x8, 4 << 1);
    game.write_ansi(entry + 0x10, "TdPawn");

    const auto entries = game.allocate(0x40);
    game.write<std::uintptr_t>(entries + 4 * sizeof(std::uintptr_t), entry);

    const auto g_names = game.allocate(0x10);
    game.write<std::uintptr_t>(g_names, entries);

    const auto actor = game.allocate(0x40);
    game.write<std::uint32_t>(actor + 0x28, 4);
    game.write<std::uint32_t>(actor + 0x2C, 8);

    const auto name = get_actor_name(actor, g_names, NameLayout::ue3());

    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(*name, "TdPawn_7");
}

TEST(unit_test_unreal_actor_name, ue2_5_pointer_array_has_no_instance_number)
{
    FakeGame game;

    const auto entry = game.allocate(0x40);
    game.write_ansi(entry + 0x14, "Pawn");

    const auto entries = game.allocate(0x40);
    game.write<std::uintptr_t>(entries + 2 * sizeof(std::uintptr_t), entry);

    const auto g_names = game.allocate(0x10);
    game.write<std::uintptr_t>(g_names, entries);

    const auto actor = game.allocate(0x40);
    game.write<std::uint32_t>(actor + 0x28, 2);
    // UE 2.5 FName is a bare index, whatever follows it must not become a "_N" suffix
    game.write<std::uint32_t>(actor + 0x2C, 777);

    const auto name = get_actor_name(actor, g_names, NameLayout::ue2_5());

    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(*name, "Pawn");
}

TEST(unit_test_unreal_actor_name, ue2_5_unicode_build)
{
    FakeGame game;

    const auto entry = game.allocate(0x40);
    game.write_wide(entry + 0x14, u"Pawn");

    const auto entries = game.allocate(0x40);
    game.write<std::uintptr_t>(entries, entry);

    const auto g_names = game.allocate(0x10);
    game.write<std::uintptr_t>(g_names, entries);

    const auto actor = game.allocate(0x40);
    game.write<std::uint32_t>(actor + 0x28, 0);

    auto layout = NameLayout::ue2_5();
    layout.char_kind = NameCharKind::WIDE;

    const auto name = get_actor_name(actor, g_names, layout);

    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(*name, "Pawn");
}

TEST(unit_test_unreal_actor_name, null_arguments_are_rejected)
{
    FakeGame game;

    const auto actor = game.allocate(0x30);

    EXPECT_EQ(get_actor_name(0, actor, NameLayout::ue5()).error(), ActorNameError::NULL_POINTER);
    EXPECT_EQ(get_actor_name(actor, 0, NameLayout::ue5()).error(), ActorNameError::NULL_POINTER);
}

TEST(unit_test_unreal_actor_name, missing_block_is_reported)
{
    FakeGame game;

    const auto g_names = game.allocate(0x30);
    const auto actor = game.allocate(0x30);
    game.write<std::uint32_t>(actor + 0x18, 2u << 16);

    EXPECT_EQ(get_actor_name(actor, g_names, NameLayout::ue5()).error(), ActorNameError::INVALID_NAME_INDEX);
}

TEST(unit_test_unreal_actor_name, missing_entry_is_reported)
{
    FakeGame game;

    const auto chunk = game.allocate(0x40);
    const auto g_names = game.allocate(0x40);
    game.write<std::uintptr_t>(g_names, chunk);

    const auto actor = game.allocate(0x30);
    game.write<std::uint32_t>(actor + 0x18, 5);

    EXPECT_EQ(get_actor_name(actor, g_names, NameLayout::ue4_legacy()).error(), ActorNameError::INVALID_NAME_INDEX);
}

TEST(unit_test_unreal_actor_name, empty_text_is_reported)
{
    FakeGame game;

    const auto entry = game.allocate(0x40);
    const auto chunk = game.allocate(0x40);
    game.write<std::uintptr_t>(chunk, entry);

    const auto g_names = game.allocate(0x40);
    game.write<std::uintptr_t>(g_names, chunk);

    const auto actor = game.allocate(0x30);
    game.write<std::uint32_t>(actor + 0x18, 0);

    EXPECT_EQ(get_actor_name(actor, g_names, NameLayout::ue4_legacy()).error(), ActorNameError::EMPTY_NAME);
}

TEST(unit_test_unreal_actor_name, null_terminated_name_is_capped)
{
    FakeGame game;

    const auto entry = game.allocate(0x40);
    game.write_ansi(entry + 0x10, "ThisNameIsNotTerminated");

    const auto chunk = game.allocate(0x40);
    game.write<std::uintptr_t>(chunk, entry);

    const auto g_names = game.allocate(0x40);
    game.write<std::uintptr_t>(g_names, chunk);

    const auto actor = game.allocate(0x30);
    game.write<std::uint32_t>(actor + 0x18, 0);

    auto layout = NameLayout::ue4_legacy();
    layout.max_name_length = 8;

    const auto name = get_actor_name(actor, g_names, layout);

    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(*name, "ThisName");
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

TEST(unit_test_unreal_actor_name, ue3_32_bit_target_read_externally)
{
    g_target_memory.assign(0x1000, std::byte{});

    constexpr std::uintptr_t g_names = 0x100;
    constexpr std::uintptr_t entries = 0x200;
    constexpr std::uintptr_t entry = 0x300;
    constexpr std::uintptr_t actor = 0x400;

    write_target<std::uint32_t>(g_names, entries);
    write_target<std::uint32_t>(entries + 3 * sizeof(std::uint32_t), entry);
    write_target<std::int32_t>(entry + 0x8, 3 << 1);
    std::memcpy(g_target_memory.data() + entry + 0x10, "PlayerPawn", 10);
    write_target<std::uint32_t>(actor + 0x28, 3);
    write_target<std::uint32_t>(actor + 0x2C, 5);

    auto layout = NameLayout::ue3();
    layout.pointer_size = sizeof(std::uint32_t);

    const auto name = get_actor_name<FakeExternalMemoryTrait>(actor, g_names, layout);

    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(*name, "PlayerPawn_4");
}
