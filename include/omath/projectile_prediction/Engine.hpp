//
// Created by Vlad on 6/9/2024.
//

#pragma once

#include <optional>
#include "omath/Vector3.hpp"
#include "omath/projectile_prediction/Projectile.hpp"
#include "omath/projectile_prediction/Target.hpp"

namespace omath::projectile_prediction
{
    class Engine final
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


        [[nodiscard]] static std::optional<float> CalculatePitch(const Vector3 &projOrigin, const Vector3 &targetPos,
                                            float bulletGravity, float v0, float time);
    };
}