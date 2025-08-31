//
// Created by Vlad on 2/23/2025.
//
#pragma once
#include "omath/linear_algebra/vector3.hpp"
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/projectile_prediction/target.hpp"

namespace omath::projectile_prediction
{
    class ProjPredEngineInterface
    {
    public:
        [[nodiscard]]
        virtual std::optional<Vector3<float>> maybe_calculate_aim_point(const Projectile& projectile,
                                                                        const Target& target) const = 0;
        virtual ~ProjPredEngineInterface() = default;
    };
} // namespace omath::projectile_prediction
