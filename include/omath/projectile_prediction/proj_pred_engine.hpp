//
// Created by Vlad on 2/23/2025.
//
#pragma once
#include "Projectile.hpp"
#include "Target.hpp"
#include "omath/Vector3.hpp"


namespace omath::projectile_prediction
{
    class ProjPredEngine
    {
    public:
        [[nodiscard]]
        virtual std::optional<Vector3<float>> MaybeCalculateAimPoint(const Projectile& projectile,
                                                              const Target& target) const = 0;
        virtual ~ProjPredEngine() = default;
    };
} // namespace omath::projectile_prediction
