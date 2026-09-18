//
// Created by Vlad on 2/23/2025.
//
#pragma once
#include "omath/projectile_prediction/proj_pred_engine.hpp"

namespace omath::projectile_prediction
{
    // Source-style solver (z up, yaw about z) that scans candidate flight times eight at a time with AVX2. Only
    // available when the library is built with OMATH_USE_AVX2 on x86; every call throws otherwise.
    class ProjPredEngineAvx2 final : public ProjPredEngineInterface<float>
    {
    public:
        ProjPredEngineAvx2(float gravity_constant, float simulation_time_step, float maximum_simulation_time) noexcept;

        [[nodiscard]]
        std::optional<AimSolution<float>> maybe_calculate_aim(const Projectile<float>& projectile,
                                                              const Launcher<float>& launcher,
                                                              const Target<float>& target) const override;

    private:
        struct Candidate
        {
            float time;
            Vector3<float> target_position;
            float pitch;
        };

        // First flight time at which the projectile can reach the predicted target from launch_origin
        [[nodiscard]]
        std::optional<Candidate> find_candidate(const Vector3<float>& launch_origin, const Target<float>& target,
                                                float bullet_gravity, float launch_speed) const;

        [[nodiscard]]
        static std::optional<float> calculate_pitch(const Vector3<float>& launch_origin,
                                                    const Vector3<float>& target_pos, float bullet_gravity,
                                                    float launch_speed, float time);

        // [[maybe_unused]] because the members are only read on x86 with AVX2 enabled
        [[maybe_unused]] float m_gravity_constant;
        [[maybe_unused]] float m_simulation_time_step;
        [[maybe_unused]] float m_maximum_simulation_time;
    };
} // namespace omath::projectile_prediction
