//
// Created by orange on 15.03.2026.
//
#pragma once
#include "omath/linear_algebra/vector2.hpp"
#include "omath/utility/color.hpp"
#include <optional>
#include <string>

namespace omath::hud::widget
{
    // ── Boxes ─────────────────────────────────────────────────────────────────
    struct Box
    {
        Color color;
        Color fill{0.f, 0.f, 0.f, 0.f};
        float thickness = 1.f;
    };

    struct CorneredBox
    {
        Color color;
        Color fill{0.f, 0.f, 0.f, 0.f};
        float corner_ratio = 0.2f;
        float thickness = 1.f;
    };

    struct DashedBox
    {
        Color color;
        float dash_len = 8.f;
        float gap_len = 5.f;
        float thickness = 1.f;
    };

    // ── Bars ──────────────────────────────────────────────────────────────────
    struct RightBar
    {
        Color color;
        Color outline;
        Color bg;
        float width;
        float ratio;
        float offset = 5.f;
    };
    struct LeftBar
    {
        Color color;
        Color outline;
        Color bg;
        float width;
        float ratio;
        float offset = 5.f;
    };
    struct TopBar
    {
        Color color;
        Color outline;
        Color bg;
        float height;
        float ratio;
        float offset = 5.f;
    };
    struct BottomBar
    {
        Color color;
        Color outline;
        Color bg;
        float height;
        float ratio;
        float offset = 5.f;
    };

    struct RightDashedBar
    {
        Color color;
        Color outline;
        Color bg;
        float width;
        float ratio;
        float dash_len;
        float gap_len;
        float offset = 5.f;
    };
    struct LeftDashedBar
    {
        Color color;
        Color outline;
        Color bg;
        float width;
        float ratio;
        float dash_len;
        float gap_len;
        float offset = 5.f;
    };
    struct TopDashedBar
    {
        Color color;
        Color outline;
        Color bg;
        float height;
        float ratio;
        float dash_len;
        float gap_len;
        float offset = 5.f;
    };
    struct BottomDashedBar
    {
        Color color;
        Color outline;
        Color bg;
        float height;
        float ratio;
        float dash_len;
        float gap_len;
        float offset = 5.f;
    };

    // ── Labels ────────────────────────────────────────────────────────────────
    struct RightLabel
    {
        Color color;
        float offset;
        bool outlined;
        std::string text;
    };
    struct LeftLabel
    {
        Color color;
        float offset;
        bool outlined;
        std::string text;
    };
    struct TopLabel
    {
        Color color;
        float offset;
        bool outlined;
        std::string text;
    };
    struct BottomLabel
    {
        Color color;
        float offset;
        bool outlined;
        std::string text;
    };
    struct CenteredTopLabel
    {
        Color color;
        float offset;
        bool outlined;
        std::string text;
    };
    struct CenteredBottomLabel
    {
        Color color;
        float offset;
        bool outlined;
        std::string text;
    };

    // ── Misc ──────────────────────────────────────────────────────────────────
    struct Skeleton
    {
        Color color;
        float thickness = 1.f;
    };
    struct SnapLine
    {
        Vector2<float> start;
        Color color;
        float width;
    };

} // namespace omath::hud::widget

namespace omath::hud
{
    /// Returns an engaged optional<W> when condition is true, std::nullopt otherwise.
    /// Designed for use with EntityOverlay::contents() to conditionally include widgets.
    template<typename W>
    std::optional<W> when(bool condition, W widget)
    {
        if (condition)
            return std::move(widget);
        return std::nullopt;
    }
} // namespace omath::hud
