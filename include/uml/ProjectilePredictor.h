//
// Created by vlad on 11/6/23.
//

#pragma once
#include <optional>
#include <uml/Vector3.h>


namespace uml::prediction
{
    struct Projectile
    {
        Vector3 m_origin;
        float m_velocity{};
        float m_gravityMultiplier = 1.f;
    };
    struct Target
    {
        Vector3 m_origin;
        Vector3 m_vecVelocity;
        bool m_IsAirborne;
    };
    class ProjectilePredictor
    {
    public:
        explicit ProjectilePredictor(float gravityValue,
                                     float maxTimeToTravel = 3.f,
                                     float timeStep = 0.1f);


        [[nodiscard]]
        std::optional<Vector3> PredictPointToAim(const Target& target,
                                                 const Projectile& projectile) const;

    private:
        const float m_gravity;
        const float m_maxTravelTime;
        const float m_timeStepSize;

        [[nodiscard]]
        Vector3 LinearPrediction(const Target& target, float time) const;

        [[nodiscard]]
        std::optional<float>
        MaybeCalculateProjectileLaunchPitchAngle(const Projectile& projectile,
                                                 const Vector3& targetPosition) const;

        [[nodiscard]]
        std::optional<float> ProjectileTravelTime(const Vector3& end,
                                                  const Projectile& projectile,
                                                  float angle) const;
    };

};
