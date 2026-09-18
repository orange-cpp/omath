//
// Created by Orange on 9/18/2026.
//

#pragma once
#include "omath/linear_algebra/vector3.hpp"

namespace omath::projectile_prediction
{
    // Where the muzzle sits relative to the eye, in the shooter's own view frame. Engines spawn a projectile at
    // eye + forward * a + right * b + up * c, so its world-space position rotates with the view angles.
    template<class ArithmeticType = float>
    struct MuzzleOffset final
    {
        ArithmeticType forward{};
        ArithmeticType right{};
        ArithmeticType up{};

        [[nodiscard("You must use zero check result")]]
        constexpr bool is_zero() const noexcept
        {
            return forward == ArithmeticType{0} && right == ArithmeticType{0} && up == ArithmeticType{0};
        }
    };

    // Forward, right and up of a view frame for a given pitch and yaw, expressed in the engine's world axes.
    template<class ArithmeticType = float>
    struct ViewBasis final
    {
        Vector3<ArithmeticType> forward;
        Vector3<ArithmeticType> right;
        Vector3<ArithmeticType> up;
    };

    // Who fires: the eye the view angles apply to, and where the projectile leaves relative to it.
    template<class ArithmeticType = float>
    struct Launcher final
    {
        Vector3<ArithmeticType> eye_origin;
        MuzzleOffset<ArithmeticType> muzzle_offset{};

        // Fixed world-space part of the spawn offset, for engines whose offset does not rotate with the view. The
        // compatibility wrappers of ProjPredEngineInterface put the old Projectile::m_launch_offset here.
        Vector3<ArithmeticType> world_offset{};

        [[nodiscard("You must use launch origin")]]
        constexpr Vector3<ArithmeticType> launch_origin(const ViewBasis<ArithmeticType>& basis) const noexcept
        {
            return eye_origin + world_offset + basis.forward * muzzle_offset.forward + basis.right * muzzle_offset.right
                   + basis.up * muzzle_offset.up;
        }
    };
} // namespace omath::projectile_prediction
