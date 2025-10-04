//
// Created by Vlad on 10/4/2025.
//
#include "omath/linear_algebra/vector3.hpp"
#include <gtest/gtest.h>
#include <omath/rev_eng/internal_rev_object.hpp>

class Player final
{
public:
    virtual int foo() {return 1;}
    virtual int bar() {return 2;}
    omath::Vector3<float> m_origin{1.f, 2.f, 3.f};
    int m_health{123};
};

class RevPlayer : omath::rev_eng::InternalReverseEngineeredObject
{
public:
    omath::Vector3<float> get_origin()
    {
        return get_by_offset<omath::Vector3<float>>(sizeof(std::uintptr_t));
    }
    int get_health()
    {
        return get_by_offset<int>(sizeof(std::uintptr_t)+sizeof(omath::Vector3<float>));
    }

    int rev_foo()
    {
        return call_virtual_method<0, int>();
    }
    int rev_bar()
    {
        return call_virtual_method<1, int>();
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
}