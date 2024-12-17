//
// Created by Vlad on 6/9/2024.
//

#pragma once

#include <optional>
#include "omath/Vector3.hpp"
#include "omath/prediction/Projectile.hpp"
#include "omath/prediction/Target.hpp"
#include "omath/omath_export.hpp"

namespace omath::prediction
{
    class OMATH_API Engine final
    {
    public:
        explicit Engine(float gravityConstant, float simulationTimeStep,
                        float maximumSimulationTime, float distanceTolerance);

        [[nodiscard]]
        std::optional<Vector3> MaybeCalculateAimPoint(const Projectile& projectile, const Target& target) const;

    private:
        const float m_gravityConstant;
        const float m_simulationTimeStep;
        const float m_maximumSimulationTime;
        const float m_distanceTolerance;

        [[nodiscard]]
        std::optional<float> MaybeCalculateProjectileLaunchPitchAngle(const Projectile& projectile,
                                                                      const Vector3& targetPosition) const;


        [[nodiscard]]
        bool IsProjectileReachedTarget(const Vector3& targetPosition, const Projectile& projectile, float pitch, float time) const;

    };
}