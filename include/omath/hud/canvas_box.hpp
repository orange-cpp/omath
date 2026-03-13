//
// Created by orange on 13.03.2026.
//
#pragma once
#include "omath/linear_algebra/vector2.hpp"

namespace omath::hud
{
    class CanvasBox final
    {
    public:
        CanvasBox(Vector2<float> top, Vector2<float> bottom, float ratio = 4.f);

        [[nodiscard]]
        std::array<Vector2<float>, 4> as_array() const;

        Vector2<float> top_left_corner;
        Vector2<float> top_right_corner;

        Vector2<float> bottom_left_corner;
        Vector2<float> bottom_right_corner;
    };
} // namespace omath::hud