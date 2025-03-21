//
// Created by Vlad on 6/9/2024.
//

#pragma once

#include <optional>
#include "omath/projectile_prediction/proj_pred_engine.hpp"
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/projectile_prediction/target.hpp"
#include "omath/vector3.hpp"


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
