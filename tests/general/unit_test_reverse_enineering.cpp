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

// Helper to extract a member function pointer address as const void*
template<typename T>
const void* member_fn_to_ptr(T fn)
{
    union
    {
        T member;
        const void* ptr;
    } u{};
    static_assert(sizeof(T) == sizeof(const void*), "Only simple member function pointers supported");
    u.member = fn;
    return u.ptr;
}

// Target class with non-virtual member functions for call_method testing
class MethodTarget
{
public:
    int add(int a, int b) { return a + b; }
    float scale(float val, float factor) { return val * factor; }
    int get_42() const { return 42; }
};

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

    // Wrappers exposing call_method for testing with real member function addresses
    int call_add(int a, int b)
    {
        return call_method<int>(member_fn_to_ptr(&MethodTarget::add), a, b);
    }

    float call_scale(float val, float factor)
    {
        return call_method<float>(member_fn_to_ptr(&MethodTarget::scale), val, factor);
    }

    int call_get_42() const
    {
        return call_method<int>(member_fn_to_ptr(&MethodTarget::get_42));
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
    MethodTarget target;
    auto* rev = reinterpret_cast<RevPlayer*>(&target);

    EXPECT_EQ(target.add(3, 4), rev->call_add(3, 4));
    EXPECT_EQ(7, rev->call_add(3, 4));
}

TEST(unit_test_reverse_enineering, call_method_float_args)
{
    MethodTarget target;
    auto* rev = reinterpret_cast<RevPlayer*>(&target);

    EXPECT_FLOAT_EQ(6.0f, rev->call_scale(2.0f, 3.0f));
    EXPECT_FLOAT_EQ(0.0f, rev->call_scale(0.0f, 100.0f));
    EXPECT_FLOAT_EQ(-5.0f, rev->call_scale(5.0f, -1.0f));
}

TEST(unit_test_reverse_enineering, call_method_const)
{
    MethodTarget target;
    const auto* rev = reinterpret_cast<const RevPlayer*>(&target);

    EXPECT_EQ(42, rev->call_get_42());
}

TEST(unit_test_reverse_enineering, call_method_no_extra_args)
{
    MethodTarget target;
    const auto* rev = reinterpret_cast<const RevPlayer*>(&target);

    // call_get_42 takes no arguments beyond this — verifies zero-arg pack works
    EXPECT_EQ(42, rev->call_get_42());
}

TEST(unit_test_reverse_enineering, call_virtual_method_delegates_to_call_method)
{
    // call_virtual_method now internally uses call_method — verify both vtable slots
    Player player_original;
    auto* player_reversed = reinterpret_cast<RevPlayer*>(&player_original);

    EXPECT_EQ(1, player_reversed->rev_foo());
    EXPECT_EQ(2, player_reversed->rev_bar());
}