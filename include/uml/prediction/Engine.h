//
// Created by Vlad on 6/9/2024.
//

#pragma once

#include <optional>
#include "uml/Vector3.h"
#include "uml/prediction/Projectile.h"
#include "uml/prediction/Target.h"


namespace uml::prediction
{
    class Engine
    {
    public:
        explicit Engine(float gravityConstant, float simulationTimeStep, float maximumSimulationTime);

        [[nodiscard]]
        std::optional<Vector3> MaybeCalculateAimPoint(const Projectile& projectile, const Target& target) const;

    private:
        const float m_gravityConstant;
        const float m_simulationTimeStep;
        const float m_maximumSimulationTime;

        [[nodiscard]]
        std::optional<float> MaybeCalculateProjectileLaunchPitchAngle(const Projectile& projectile,
                                                                      const Vector3& targetPosition) const;


        [[nodiscard]]
        bool IsProjectileReachedTarget(const Vector3& targetPosition, const Projectile& projectile, float pitch, float time) const;

    };
}