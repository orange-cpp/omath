//
// Created by Vlad on 17.09.2024.
//
#include <gtest/gtest.h>
#include <omath/hooking/subhook.h>

int foo(int x)
{
    for (int i = 0; i < 3; i++)
    {
        if (i == 1)
            x *= 2;

        x += i * i + 1;
        x /= i;
    }
    return x;
}
int fooDetour(int x)
{
    return -1;
}
TEST(UnitTestHooking, EqTest)
{
    omath::hooking::Hook hk(foo, fooDetour, omath::hooking::HookFlags::HookFlag64BitOffset | omath::hooking::HookFlags::HookFlagTrampoline);

    EXPECT_TRUE(hk.GetTrampoline());
}