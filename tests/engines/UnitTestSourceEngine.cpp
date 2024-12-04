//
// Created by Orange on 11/23/2024.
//
#include <gtest/gtest.h>
#include <omath/engines/Source/Constants.h>
#include <omath/engines/Source/Formulas.hpp>





TEST(UnitTestSourceEngine, ForwardVector)
{
    const auto forward = omath::source::ForwardVector({});

    EXPECT_EQ(forward, omath::source::kAbsForward);
}

TEST(UnitTestSourceEngine, RightVector)
{
    const auto right = omath::source::RightVector({});

    EXPECT_EQ(right, omath::source::kAbsRight);
}

TEST(UnitTestSourceEngine, UpVector)
{
    const auto up = omath::source::UpVector({});
    EXPECT_EQ(up, omath::source::kAbsUp);
}