//
// Created by Vlad on 6/9/2024.
//


#include "omath/prediction/Engine.h"
#include <cmath>
#include <omath/Angles.h>


namespace omath::prediction
{
    Engine::Engine(const float gravityConstant, const float simulationTimeStep,
        const float maximumSimulationTime, const float distanceTolerance)
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

            const auto delta = (predictedTargetPosition - projectile.m_origin);

#if OMATH_COORDINATE_SYSTEM == OMATH_QUAKE_SUPPORT
            const auto horizontalDistance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
#elif OMATH_COORDINATE_SYSTEM == OMATH_UE_SUPPORT
            const auto horizontalDistance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
#elif OMATH_COORDINATE_SYSTEM == OMATH_UOMATH_UNITY_SUPPORT
            const auto horizontalDistance = std::sqrt(delta.x * delta.x + delta.z * delta.z);
#endif
            const auto height = horizontalDistance * std::tan(angles::DegreesToRadians(projectilePitch.value()));

#if OMATH_COORDINATE_SYSTEM == OMATH_QUAKE_SUPPORT
            return Vector3(predictedTargetPosition.x, predictedTargetPosition.y, projectile.m_origin.z + height);
#elif OMATH_COORDINATE_SYSTEM == OMATH_UE_SUPPORT
            return Vector3(predictedTargetPosition.x, predictedTargetPosition.y, projectile.m_origin.z + height);
#elif OMATH_COORDINATE_SYSTEM == OMATH_UOMATH_UNITY_SUPPORT
            return Vector3(predictedTargetPosition.x, predictedTargetPosition.y + height, projectile.m_origin.z);
#endif
        }
        return std::nullopt;
    }

    std::optional<float> Engine::MaybeCalculateProjectileLaunchPitchAngle(const Projectile &projectile,
        const Vector3 &targetPosition) const
    {
        const auto bulletGravity = m_gravityConstant * projectile.m_gravityScale;
        const auto delta = targetPosition - projectile.m_origin;

#if OMATH_COORDINATE_SYSTEM == OMATH_QUAKE_SUPPORT
        const auto horizontalDistance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        const auto verticalDistance = delta.z;
#elif OMATH_COORDINATE_SYSTEM == OMATH_UE_SUPPORT
        const auto horizontalDistance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        const auto verticalDistance = delta.z;
#elif OMATH_COORDINATE_SYSTEM == OMATH_UOMATH_UNITY_SUPPORT
        const auto horizontalDistance = std::sqrt(delta.x * delta.x + delta.z * delta.z);
        const auto verticalDistance = delta.y;
#endif
        const auto projSpeedSqr = projectile.m_launchSpeed * projectile.m_launchSpeed;

        const float underRoot = projSpeedSqr*projSpeedSqr - bulletGravity * (bulletGravity *
        (horizontalDistance*horizontalDistance) + 2.0f * verticalDistance * projSpeedSqr);

        if (underRoot < 0.0f) [[unlikely]]
            return std::nullopt;

        const float root = std::sqrt(underRoot);
        const float angle = std::atan((projSpeedSqr - root) / (bulletGravity * horizontalDistance));

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
