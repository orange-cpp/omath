//
// Created by Vladislav on 19.02.2026.
//
#include <gtest/gtest.h>
#include <omath/engines/cry_engine/camera.hpp>
#include <omath/engines/cry_engine/constants.hpp>
#include <omath/engines/cry_engine/formulas.hpp>
#include <random>
#include <ranges>

using namespace omath;
TEST(unit_test_cry_engine, look_at_forward)
{
    const auto angles = cry_engine::CameraTrait::calc_look_at_angle({}, cry_engine::k_abs_forward);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = cry_engine::forward_vector(angles);
    for (const auto& [result, etalon] : std::views::zip(dir_vector.as_array(), cry_engine::k_abs_forward.as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}
TEST(unit_test_cry_engine, look_at_right)
{
    const auto angles = cry_engine::CameraTrait::calc_look_at_angle({}, cry_engine::k_abs_right);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = cry_engine::forward_vector(angles);
    for (const auto& [result, etalon] : std::views::zip(dir_vector.as_array(), cry_engine::k_abs_right.as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}
TEST(unit_test_cry_engine, look_at_up)
{
    const auto angles = cry_engine::CameraTrait::calc_look_at_angle({}, cry_engine::k_abs_right);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = cry_engine::forward_vector(angles);
    for (const auto& [result, etalon] : std::views::zip(dir_vector.as_array(), cry_engine::k_abs_right.as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}

TEST(unit_test_cry_engine, look_at_back)
{
    const auto angles = cry_engine::CameraTrait::calc_look_at_angle({}, -cry_engine::k_abs_forward);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = cry_engine::forward_vector(angles);
    for (const auto& [result, etalon] : std::views::zip(dir_vector.as_array(), (-cry_engine::k_abs_forward).as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}
TEST(unit_test_cry_engine, look_at_left)
{
    const auto angles = cry_engine::CameraTrait::calc_look_at_angle({}, -cry_engine::k_abs_right);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = cry_engine::forward_vector(angles);
    for (const auto& [result, etalon] : std::views::zip(dir_vector.as_array(), (-cry_engine::k_abs_right).as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}
TEST(unit_test_cry_engine, look_at_down)
{
    const auto angles = cry_engine::CameraTrait::calc_look_at_angle({}, -cry_engine::k_abs_up);

    // ReSharper disable once CppTooWideScopeInitStatement
    const auto dir_vector = cry_engine::forward_vector(angles);
    for (const auto& [result, etalon] : std::views::zip(dir_vector.as_array(), (-cry_engine::k_abs_up).as_array()))
        EXPECT_NEAR(result, etalon, 0.0001f);
}