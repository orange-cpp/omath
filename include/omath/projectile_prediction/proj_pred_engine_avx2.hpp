//
// Created by Vlad on 2/23/2025.
//
#pragma once
#include "omath/projectile_prediction/proj_pred_engine.hpp"

namespace omath::projectile_prediction
{
    class ProjPredEngineAvx2 final : public ProjPredEngineInterface
    {
    public:
        [[nodiscard]] std::optional<Vector3<float>>
        maybe_calculate_aim_point(const Projectile& projectile, const Target& target) const override;

        ProjPredEngineAvx2(float gravity_constant, float simulation_time_step, float maximum_simulation_time);
        ~ProjPredEngineAvx2() override = default;

    private:
        [[nodiscard]] static std::optional<float> calculate_pitch(const Vector3<float>& proj_origin,
                                                                  const Vector3<float>& target_pos,
                                                                  float bullet_gravity, float v0, float time) ;

        // We use [[maybe_unused]] here since AVX2 is not available for ARM and ARM64 CPU
        [[maybe_unused]] const float m_gravity_constant;
        [[maybe_unused]] const float m_simulation_time_step;
        [[maybe_unused]] const float m_maximum_simulation_time;
    };
} // namespace omath::projectile_prediction
