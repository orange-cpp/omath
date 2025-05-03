//
// Created by Vlad on 6/9/2024.
//

#pragma once

#include "omath/projectile_prediction/proj_pred_engine.hpp"
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/projectile_prediction/target.hpp"
#include "omath/vector3.hpp"
#include <optional>

namespace omath::projectile_prediction
{
    class ProjPredEngineLegacy final : public ProjPredEngine
    {
    public:
        explicit ProjPredEngineLegacy(float gravity_constant, float simulation_time_step, float maximum_simulation_time,
                                      float distance_tolerance);

        [[nodiscard]]
        std::optional<Vector3<float>> maybe_calculate_aim_point(const Projectile& projectile,
                                                                const Target& target) const override;

    private:
        const float m_gravity_constant;
        const float m_simulation_time_step;
        const float m_maximum_simulation_time;
        const float m_distance_tolerance;

        [[nodiscard]]
        std::optional<float> maybe_calculate_projectile_launch_pitch_angle(const Projectile& projectile,
                                                                           const Vector3<float>& target_position) const;

        [[nodiscard]]
        bool is_projectile_reached_target(const Vector3<float>& target_position, const Projectile& projectile,
                                          float pitch, float time) const;
    };
} // namespace omath::projectile_prediction
