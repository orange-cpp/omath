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

// Free functions that mimic member function calling convention (this as first arg)
inline int free_add(void* /*this_ptr*/, int a, int b) { return a + b; }
inline float free_scale(void* /*this_ptr*/, float val, float factor) { return val * factor; }
inline int free_get_42(const void* /*this_ptr*/) { return 42; }

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

    // Wrappers exposing call_method for testing
    int call_add(int a, int b)
    {
        return call_method<int>(reinterpret_cast<const void*>(&free_add), a, b);
    }

    float call_scale(float val, float factor)
    {
        return call_method<float>(reinterpret_cast<const void*>(&free_scale), val, factor);
    }

    int call_get_42() const
    {
        return call_method<int>(reinterpret_cast<const void*>(&free_get_42));
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

TEST(unit_test_reverse_enineering, call_method_with_args)
{
    Player player_original;
    auto* player_reversed = reinterpret_cast<RevPlayer*>(&player_original);

    EXPECT_EQ(free_add(nullptr, 3, 4), player_reversed->call_add(3, 4));
    EXPECT_EQ(7, player_reversed->call_add(3, 4));
}

TEST(unit_test_reverse_enineering, call_method_float_args)
{
    Player player_original;
    auto* player_reversed = reinterpret_cast<RevPlayer*>(&player_original);

    EXPECT_FLOAT_EQ(6.0f, player_reversed->call_scale(2.0f, 3.0f));
    EXPECT_FLOAT_EQ(0.0f, player_reversed->call_scale(0.0f, 100.0f));
    EXPECT_FLOAT_EQ(-5.0f, player_reversed->call_scale(5.0f, -1.0f));
}

TEST(unit_test_reverse_enineering, call_method_const)
{
    Player player_original;
    const auto* player_reversed = reinterpret_cast<const RevPlayer*>(&player_original);

    EXPECT_EQ(42, player_reversed->call_get_42());
}

TEST(unit_test_reverse_enineering, call_method_no_extra_args)
{
    Player player_original;
    const auto* player_reversed = reinterpret_cast<const RevPlayer*>(&player_original);

    // call_get_42 takes no arguments beyond this — verifies zero-arg pack works
    EXPECT_EQ(42, player_reversed->call_get_42());
}

TEST(unit_test_reverse_enineering, call_virtual_method_delegates_to_call_method)
{
    // call_virtual_method now internally uses call_method — verify both vtable slots
    Player player_original;
    auto* player_reversed = reinterpret_cast<RevPlayer*>(&player_original);

    EXPECT_EQ(1, player_reversed->rev_foo());
    EXPECT_EQ(2, player_reversed->rev_bar());
}