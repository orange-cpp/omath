//
// Created by orange on 15.03.2026.
//
#pragma once
#include "omath/linear_algebra/vector2.hpp"
#include "omath/utility/color.hpp"
#include <initializer_list>
#include <optional>
#include <string_view>
#include <variant>

namespace omath::hud::widget
{
    // ── Overloaded helper for std::visit ──────────────────────────────────────
    template<typename... Ts>
    struct Overloaded : Ts...
    {
        using Ts::operator()...;
    };
    template<typename... Ts>
    Overloaded(Ts...) -> Overloaded<Ts...>;

    // ── Standalone widgets ────────────────────────────────────────────────────
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

    // ── Side-agnostic widgets (used inside XxxSide containers) ────────────────

    /// A filled bar. `size` is width for left/right sides, height for top/bottom.
    struct Bar
    {
        Color color;
        Color outline;
        Color bg;
        float size;
        float ratio;
        float offset = 5.f;
    };

    /// A dashed bar. Same field semantics as Bar plus dash parameters.
    struct DashedBar
    {
        Color color;
        Color outline;
        Color bg;
        float size;
        float ratio;
        float dash_len;
        float gap_len;
        float offset = 5.f;
    };

    struct Label
    {
        Color color;
        float offset;
        bool outlined;
        std::string_view text;
    };

    /// Wraps a Label to request horizontal centering (only applied in TopSide / BottomSide).
    template<typename W>
    struct Centered
    {
        W child;
    };
    template<typename W>
    Centered(W) -> Centered<W>;

    /// Empty vertical gap that advances the Y cursor without drawing.
    struct SpaceVertical
    {
        float size;
    };

    /// Empty horizontal gap that advances the X cursor without drawing.
    struct SpaceHorizontal
    {
        float size;
    };

    struct ProgressRing
    {
        Color color;
        Color bg{0.3f, 0.3f, 0.3f, 0.5f};
        float radius = 12.f;
        float ratio;
        float thickness = 2.f;
        float offset = 5.f;
        int segments = 32;
    };

    // ── Side widget variant ───────────────────────────────────────────────────
    struct None
    {
    }; ///< No-op placeholder — used by widget::when for disabled elements.
    using SideWidget = std::variant<None, Bar, DashedBar, Label, Centered<Label>, SpaceVertical, SpaceHorizontal, ProgressRing>;

    // ── Side containers ───────────────────────────────────────────────────────
    // Storing std::initializer_list<SideWidget> is safe here: the backing array
    // is a const SideWidget[] on the caller's stack whose lifetime matches the
    // temporary side-container object, which is consumed within the same
    // full-expression by EntityOverlay::dispatch. No heap allocation occurs.

    struct RightSide
    {
        std::initializer_list<SideWidget> children;
        RightSide(const std::initializer_list<SideWidget> c): children(c)
        {
        }
    };
    struct LeftSide
    {
        std::initializer_list<SideWidget> children;
        LeftSide(const std::initializer_list<SideWidget> c): children(c)
        {
        }
    };
    struct TopSide
    {
        std::initializer_list<SideWidget> children;
        TopSide(const std::initializer_list<SideWidget> c): children(c)
        {
        }
    };
    struct BottomSide
    {
        std::initializer_list<SideWidget> children;
        BottomSide(const std::initializer_list<SideWidget> c): children(c)
        {
        }
    };

} // namespace omath::hud::widget

namespace omath::hud::widget
{
    /// Inside XxxSide containers: returns the widget as a SideWidget when condition is true,
    /// or None{} otherwise. Preferred over hud::when for types inside the SideWidget variant.
    template<typename W>
    requires std::constructible_from<SideWidget, W>
    SideWidget when(const bool condition, W widget)
    {
        if (condition)
            return SideWidget{std::move(widget)};
        return None{};
    }
} // namespace omath::hud::widget

namespace omath::hud
{
    /// Top-level: returns an engaged optional<W> when condition is true, std::nullopt otherwise.
    /// Designed for use with EntityOverlay::contents() for top-level widget types.
    template<typename W>
    std::optional<W> when(const bool condition, W widget)
    {
        if (condition)
            return std::move(widget);
        return std::nullopt;
    }
} // namespace omath::hud
