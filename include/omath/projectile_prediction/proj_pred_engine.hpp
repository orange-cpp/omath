//
// Created by Vlad on 2/23/2025.
//
#pragma once
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/projectile_prediction/target.hpp"
#include "omath/vector3.hpp"

namespace omath::projectile_prediction
{
    class ProjPredEngine
    {
    public:
        [[nodiscard]]
        virtual std::optional<Vector3<float>> maybe_calculate_aim_point(const Projectile& projectile,
                                                                        const Target& target) const = 0;
        virtual ~ProjPredEngine() = default;
    };
} // namespace omath::projectile_prediction
