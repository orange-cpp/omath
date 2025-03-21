//
// Created by Vlad on 2/23/2025.
//
#pragma once
#include "omath/projectile_prediction/target.hpp"
#include "omath/vector3.hpp"
#include "projectile.hpp"


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
