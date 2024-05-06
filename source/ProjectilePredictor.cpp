//
// Created by vlad on 11/6/23.
//

#include "uml/ProjectilePredictor.h"
#include "uml/Vector3.h"
#include "uml/angles.h"

#include <cmath>

namespace uml::prediction
{

    std::optional<Vector3>
    ProjectilePredictor::CalculateViewAngles(const Vector3 &origin,
                                             const Vector3 &target,
                                             const Vector3 &targetVelocity,
                                             float gravity,
                                             float bulletSpeed,
                                             float bulletGravity,
                                             bool inAir)
    {
        constexpr float maxTime = 3.0f;
        constexpr float timeStep = 0.001f;

        for (float time = 0.0f; time <= maxTime; time += timeStep)
        {
            auto predPos = target + targetVelocity * time;

            if (inAir)
                predPos -= Vector3(0, 0, gravity * (time * time) * 0.5);

            const auto angle = CalculateAimPitch(origin, predPos, bulletSpeed, bulletGravity);

            if (!angle.has_value())
                return std::nullopt;

            const auto timeToHit = ProjectileTravelTime((predPos - origin).Length2D(), *angle, bulletSpeed);

            if (timeToHit > time)
                continue;


            auto viewAngles = origin.ViewAngleTo(predPos);
            viewAngles.x = angle.value();

            return viewAngles;
        }
        return std::nullopt;
    }

    std::optional<float>
    ProjectilePredictor::CalculateAimPitch(const Vector3 &origin, const Vector3 &target,
                                           float bulletSpeed, float bulletGravity)
    {
         const auto delta = target - origin;

        const auto distance = delta.Length2D();


        float root = powf(bulletSpeed, 4) - bulletGravity * (bulletGravity * distance * distance + 2.0f * delta.z * powf(bulletSpeed, 2));
        if (root < 0.0f) {
            return std::nullopt;
        }
        root = sqrt(root);
        float angle = atanf((powf(bulletSpeed, 2) - root) / (bulletGravity * distance));
        return -angles::RadToDeg(angle);
    }

    float ProjectilePredictor::ProjectileTravelTime(float distance, float angle, float speed)
    {
        return std::abs(distance / (std::cos(angles::DegToRad(angle)) * speed));
    }
}