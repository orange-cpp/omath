#include <gtest/gtest.h>

import omath;

TEST(UnitTestModule, ImportOmath)
{
    const omath::Vector2<float> vec{1.f, 2.f};

    EXPECT_FLOAT_EQ(vec.x, 1.f);
    EXPECT_FLOAT_EQ(vec.y, 2.f);
}
