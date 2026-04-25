//
// Created by Vlad on 2/23/2025.
//
#pragma once
#include "omath/linear_algebra/vector3.hpp"
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/projectile_prediction/target.hpp"

namespace omath::projectile_prediction
{
    template<class ArithmeticType = float>
    struct AimAngles
    {
        ArithmeticType pitch{};
        ArithmeticType yaw{};
    };

    template<class ArithmeticType = float>
    class ProjPredEngineInterface
    {
    public:
        [[nodiscard]]
        virtual std::optional<Vector3<ArithmeticType>> maybe_calculate_aim_point(
            const Projectile<ArithmeticType>& projectile, const Target<ArithmeticType>& target) const = 0;

        [[nodiscard]]
        virtual std::optional<AimAngles<ArithmeticType>> maybe_calculate_aim_angles(
            const Projectile<ArithmeticType>& projectile, const Target<ArithmeticType>& target) const = 0;

        virtual ~ProjPredEngineInterface() = default;
    };
} // namespace omath::projectile_prediction
