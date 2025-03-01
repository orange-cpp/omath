//
// Created by Vlad on 6/9/2024.
//

#pragma once

#include <optional>
#include "omath/Vector3.hpp"
#include "omath/projectile_prediction/ProjPredEngine.hpp"
#include "omath/projectile_prediction/Projectile.hpp"
#include "omath/projectile_prediction/Target.hpp"


namespace omath::projectile_prediction
{
    class ProjPredEngineLegacy final : public ProjPredEngine
    {
    public:
        explicit ProjPredEngineLegacy(float gravityConstant, float simulationTimeStep, float maximumSimulationTime,
                                      float distanceTolerance);

        [[nodiscard]]
        std::optional<Vector3<float>> MaybeCalculateAimPoint(const Projectile& projectile,
                                                      const Target& target) const override;

    private:
        const float m_gravityConstant;
        const float m_simulationTimeStep;
        const float m_maximumSimulationTime;
        const float m_distanceTolerance;

        [[nodiscard]]
        std::optional<float> MaybeCalculateProjectileLaunchPitchAngle(const Projectile& projectile,
                                                                      const Vector3<float>& targetPosition) const;


        [[nodiscard]]
        bool IsProjectileReachedTarget(const Vector3<float>& targetPosition, const Projectile& projectile, float pitch,
                                       float time) const;
    };
} // namespace omath::projectile_prediction
