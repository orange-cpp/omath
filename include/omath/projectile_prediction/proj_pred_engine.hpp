//
// Created by Vlad on 2/23/2025.
//
#pragma once
#include "omath/linear_algebra/vector3.hpp"
#include "omath/projectile_prediction/launcher.hpp"
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/projectile_prediction/target.hpp"
#include <optional>

namespace omath::projectile_prediction
{
    // View angles in degrees. Pitch is positive upwards whatever the engine's own sign convention is.
    template<class ArithmeticType = float>
    struct AimAngles
    {
        ArithmeticType pitch{};
        ArithmeticType yaw{};
    };

    template<class ArithmeticType = float>
    struct AimSolution final
    {
        // View angles to set on the eye. The muzzle shares the eye's orientation, so these are the launch angles too.
        AimAngles<ArithmeticType> angles;
        // A point on the eye's aim ray, as far away as the predicted target. Any point on that ray projects to the same
        // pixel, so this is where a crosshair or marker belongs on screen.
        Vector3<ArithmeticType> aim_point;
        Vector3<ArithmeticType> predicted_target_position;
        ArithmeticType time_of_flight{};
    };

    template<class ArithmeticType = float>
    class ProjPredEngineInterface
    {
    public:
        [[nodiscard]]
        virtual std::optional<AimSolution<ArithmeticType>>
        maybe_calculate_aim(const Projectile<ArithmeticType>& projectile, const Launcher<ArithmeticType>& launcher,
                            const Target<ArithmeticType>& target) const = 0;

        // Compatibility wrappers over maybe_calculate_aim(). They read the eye from projectile.m_origin and treat
        // projectile.m_launch_offset as a fixed world-space offset, which is how the engines behaved before Launcher
        // existed. Kept for one release; new code should call maybe_calculate_aim() with a Launcher.
        [[nodiscard]]
        std::optional<Vector3<ArithmeticType>> maybe_calculate_aim_point(const Projectile<ArithmeticType>& projectile,
                                                                         const Target<ArithmeticType>& target) const
        {
            const auto solution = maybe_calculate_aim(projectile, launcher_from_projectile(projectile), target);
            if (!solution)
                return std::nullopt;

            return solution->aim_point;
        }

        [[nodiscard]]
        std::optional<AimAngles<ArithmeticType>>
        maybe_calculate_aim_angles(const Projectile<ArithmeticType>& projectile,
                                   const Target<ArithmeticType>& target) const
        {
            const auto solution = maybe_calculate_aim(projectile, launcher_from_projectile(projectile), target);
            if (!solution)
                return std::nullopt;

            return solution->angles;
        }

        [[nodiscard("You must use launcher")]]
        static constexpr Launcher<ArithmeticType>
        launcher_from_projectile(const Projectile<ArithmeticType>& projectile) noexcept
        {
            return {.eye_origin = projectile.m_origin, .muzzle_offset = {}, .world_offset = projectile.m_launch_offset};
        }

        virtual ~ProjPredEngineInterface() = default;

    protected:
        // Copying through a base reference would slice; derived engines stay copyable through these.
        ProjPredEngineInterface() = default;
        ProjPredEngineInterface(const ProjPredEngineInterface&) = default;
        ProjPredEngineInterface(ProjPredEngineInterface&&) = default;
        ProjPredEngineInterface& operator=(const ProjPredEngineInterface&) = default;
        ProjPredEngineInterface& operator=(ProjPredEngineInterface&&) = default;
    };
} // namespace omath::projectile_prediction
