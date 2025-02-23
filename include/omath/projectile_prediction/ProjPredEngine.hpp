//
// Created by Vlad on 2/23/2025.
//
#pragma once
#include "omath/Vector3.hpp"


namespace omath::projectile_prediction
{
    class ProjPredEngine
    {
    public:
        [[nodiscard]]
        virtual std::optional<Vector3> MaybeCalculateAimPoint(const Projectile& projectile,
                                                              const Target& target) const = 0;
        virtual ~ProjPredEngine() = default;
    };
} // namespace omath::projectile_prediction
