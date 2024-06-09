//
// Created by Vlad on 6/9/2024.
//


#include "uml/prediction/Engine.h"
#include <cmath>
#include <uml/angles.h>


namespace uml::prediction
{
    Engine::Engine(const float gravityConstant, const float simulationTimeStep,
        const float maximumSimulationTime, float distanceTolerance)
    : m_gravityConstant(gravityConstant),
    m_simulationTimeStep(simulationTimeStep),
    m_maximumSimulationTime(maximumSimulationTime),
    m_distanceTolerance(distanceTolerance)
    {
    }

    std::optional<Vector3> Engine::MaybeCalculateAimPoint(const Projectile &projectile, const Target &target) const
    {
        for (float time = 0.f; time < m_maximumSimulationTime; time += m_simulationTimeStep)
        {
            const auto predictedTargetPosition = target.PredictPosition(time, m_gravityConstant);

            const auto projectilePitch = MaybeCalculateProjectileLaunchPitchAngle(projectile, predictedTargetPosition);

            if (!projectilePitch.has_value()) [[unlikely]]
                continue;

            if (!IsProjectileReachedTarget(predictedTargetPosition, projectile, projectilePitch.value(), time))
                continue;

            const auto delta2d = (predictedTargetPosition - projectile.m_origin).Length2D();
            const auto height = delta2d * std::tan(angles::DegreesToRadians(projectilePitch.value()));

            return Vector3(predictedTargetPosition.x, predictedTargetPosition.y, projectile.m_origin.z + height);
        }
        return std::nullopt;
    }

    std::optional<float> Engine::MaybeCalculateProjectileLaunchPitchAngle(const Projectile &projectile,
        const Vector3 &targetPosition) const
    {
        const auto bulletGravity = m_gravityConstant * projectile.m_gravityScale;
        const auto delta = targetPosition - projectile.m_origin;;

        const auto distance2d = delta.Length2D();


        float root = std::pow(projectile.m_launchSpeed, 4.f) - bulletGravity * (bulletGravity *
            std::pow(distance2d, 2.f) + 2.0f * delta.z * std::pow(projectile.m_launchSpeed, 2.f));

        if (root < 0.0f) [[unlikely]]
            return std::nullopt;

        root = std::sqrt(root);
        const float angle = std::atan((std::pow(projectile.m_launchSpeed, 2.f) - root) / (bulletGravity * distance2d));

        return angles::RadiansToDegrees(angle);
    }

    bool Engine::IsProjectileReachedTarget(const Vector3 &targetPosition, const Projectile &projectile,
        const float pitch, const float time) const
    {
        const auto yaw = projectile.m_origin.ViewAngleTo(targetPosition).y;
        const auto projectilePosition = projectile.PredictPosition(pitch, yaw, time, m_gravityConstant);

        return projectilePosition.DistTo(targetPosition) <= m_distanceTolerance;
    }
}
