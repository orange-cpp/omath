//
// Created by Vladislav on 05.04.2025.
//
#include <gtest/gtest.h>
#include <omath/engines/minecraft_engine/formulas.hpp>

TEST(UnitTestMinecraftEngine, ForwardVector)
{
    const auto forward = omath::minecraft_engine::ForwardVector({});

    EXPECT_EQ(forward, omath::minecraft_engine::kAbsForward);
}

TEST(UnitTestMinecraftEngine, RightVector)
{
    const auto right = omath::minecraft_engine::RightVector({});

    EXPECT_EQ(right, omath::minecraft_engine::kAbsRight);
}

TEST(UnitTestMinecraftEngine, UpVector)
{
    const auto up = omath::minecraft_engine::UpVector({});
    EXPECT_EQ(up, omath::minecraft_engine::kAbsUp);
}