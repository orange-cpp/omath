//
// Created by vlad on 11/6/23.
//

#pragma once
#include <optional>

namespace uml
{
    class Vector3;
}

namespace uml::prediction
{
    class ProjectilePredictor
    {
    public:
        [[nodiscard]]
        static std::optional<Vector3> CalculateViewAngles(const Vector3& origin,
                                                          const Vector3& target,
                                                          const Vector3& targetVelocity,
                                                          float gravity,
                                                          float bulletSpeed,
                                                          float bulletGravity,
                                                          bool inAir);

    private:
        [[nodiscard]]
        static std::optional<float> CalculateAimPitch(const Vector3& origin,
                                                      const Vector3& target,
                                                      float bulletSpeed,
                                                      float bulletGravity);
        [[nodiscard]] static float ProjectileTravelTime(float distance, float angle, float speed);
    };

};
