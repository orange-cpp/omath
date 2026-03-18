//
// Created by Vlad on 10/4/2025.
//
#include "omath/linear_algebra/vector3.hpp"
#include <gtest/gtest.h>
#include <omath/rev_eng/internal_rev_object.hpp>

class Player final
{
public:
    [[nodiscard]] virtual int foo() const
    {
        return 1;
    }
    [[nodiscard]] virtual int bar() const
    {
        return 2;
    }
    omath::Vector3<float> m_origin{1.f, 2.f, 3.f};
    int m_health{123};
};

// Extract a raw function pointer from an object's vtable
inline const void* get_vtable_entry(const void* obj, const std::size_t index)
{
    const auto vtable = *static_cast<void* const* const*>(obj);
    return vtable[index];
}

class RevPlayer final : omath::rev_eng::InternalReverseEngineeredObject
{
public:
    [[nodiscard]]
    omath::Vector3<float> get_origin() const
    {
        return get_by_offset<omath::Vector3<float>>(sizeof(std::uintptr_t));
    }

    [[nodiscard]]
    int get_health() const
    {
        return get_by_offset<int>(sizeof(std::uintptr_t) + sizeof(omath::Vector3<float>));
    }

    [[nodiscard]]
    int rev_foo() const
    {
        return call_virtual_method<0, int>();
    }

    [[nodiscard]]
    int rev_bar() const
    {
        return call_virtual_method<1, int>();
    }

    [[nodiscard]] int rev_bar_const() const
    {
        return call_virtual_method<1, int>();
    }

    // Wrappers exposing call_method for testing — use vtable entries as known-good function pointers
    int call_foo_via_ptr(const void* fn_ptr) const
    {
        return call_method<int>(fn_ptr);
    }

    int call_bar_via_ptr(const void* fn_ptr) const
    {
        return call_method<int>(fn_ptr);
    }
};

TEST(unit_test_reverse_enineering, read_test)
{
    Player player_original;
    const auto player_reversed = reinterpret_cast<RevPlayer*>(&player_original);

    EXPECT_EQ(player_original.m_origin, player_reversed->get_origin());
    EXPECT_EQ(player_original.m_health, player_reversed->get_health());

    EXPECT_EQ(player_original.bar(), player_reversed->rev_bar());
    EXPECT_EQ(player_original.foo(), player_reversed->rev_foo());
    EXPECT_EQ(player_original.bar(), player_reversed->rev_bar_const());
}

TEST(unit_test_reverse_enineering, call_method_with_vtable_ptr)
{
    // Extract raw function pointers from Player's vtable, then call them via call_method
    Player player;
    const auto* rev = reinterpret_cast<const RevPlayer*>(&player);

    const auto* foo_ptr = get_vtable_entry(&player, 0);
    const auto* bar_ptr = get_vtable_entry(&player, 1);

    EXPECT_EQ(player.foo(), rev->call_foo_via_ptr(foo_ptr));
    EXPECT_EQ(player.bar(), rev->call_bar_via_ptr(bar_ptr));
    EXPECT_EQ(1, rev->call_foo_via_ptr(foo_ptr));
    EXPECT_EQ(2, rev->call_bar_via_ptr(bar_ptr));
}

TEST(unit_test_reverse_enineering, call_method_same_result_as_virtual)
{
    // call_virtual_method delegates to call_method — both paths must agree
    Player player;
    const auto* rev = reinterpret_cast<const RevPlayer*>(&player);

    EXPECT_EQ(rev->rev_foo(), rev->call_foo_via_ptr(get_vtable_entry(&player, 0)));
    EXPECT_EQ(rev->rev_bar(), rev->call_bar_via_ptr(get_vtable_entry(&player, 1)));
}

TEST(unit_test_reverse_enineering, call_virtual_method_delegates_to_call_method)
{
    Player player;
    auto* rev = reinterpret_cast<RevPlayer*>(&player);

    EXPECT_EQ(1, rev->rev_foo());
    EXPECT_EQ(2, rev->rev_bar());
    EXPECT_EQ(2, rev->rev_bar_const());
}