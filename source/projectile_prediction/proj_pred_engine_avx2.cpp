//
// Created by Vlad on 2/23/2025.
//
#include "omath/projectile_prediction/proj_pred_engine_avx2.hpp"
#include <source_location>
#include <stdexcept>

#if defined(OMATH_USE_AVX2) && (defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86))
#define OMATH_PROJ_PRED_AVX2_AVAILABLE 1
#include "omath/engines/source_engine/traits/pred_engine_trait.hpp"
#include <cmath>
#include <cstddef>
#include <immintrin.h>
#else
#include <format>
#endif

namespace omath::projectile_prediction
{
    ProjPredEngineAvx2::ProjPredEngineAvx2(const float gravity_constant, const float simulation_time_step,
                                           const float maximum_simulation_time) noexcept
        : m_gravity_constant(gravity_constant), m_simulation_time_step(simulation_time_step),
          m_maximum_simulation_time(maximum_simulation_time)
    {
    }

    std::optional<AimSolution<float>>
    ProjPredEngineAvx2::maybe_calculate_aim([[maybe_unused]] const Projectile<float>& projectile,
                                            [[maybe_unused]] const Launcher<float>& launcher,
                                            [[maybe_unused]] const Target<float>& target) const
    {
#ifdef OMATH_PROJ_PRED_AVX2_AVAILABLE
        using Trait = source_engine::PredEngineTrait;

        const float bullet_gravity = m_gravity_constant * projectile.m_gravity_scale;
        const float launch_speed = projectile.m_launch_speed;

        // The vectorised scan needs one fixed launch origin. With a view-relative muzzle offset the muzzle is first
        // placed from the direct angles to the target, and the solve is refined once from the angles that come out.
        const bool muzzle_rotates = !launcher.muzzle_offset.is_zero();
        auto launch_origin = launcher.eye_origin + launcher.world_offset;

        if (muzzle_rotates)
            launch_origin = launcher.launch_origin(
                    Trait::calc_view_basis(Trait::calc_direct_pitch_angle(launcher.eye_origin, target.m_origin),
                                           Trait::calc_direct_yaw_angle(launcher.eye_origin, target.m_origin)));

        const auto candidate = find_candidate(launch_origin, target, bullet_gravity, launch_speed);
        if (!candidate)
            return std::nullopt;

        float pitch = candidate->pitch;
        float yaw = Trait::calc_direct_yaw_angle(launch_origin, candidate->target_position);

        if (muzzle_rotates)
        {
            launch_origin = launcher.launch_origin(Trait::calc_view_basis(pitch, yaw));

            const auto refined_pitch = calculate_pitch(launch_origin, candidate->target_position, bullet_gravity,
                                                       launch_speed, candidate->time);
            if (!refined_pitch)
                return std::nullopt;

            pitch = *refined_pitch;
            yaw = Trait::calc_direct_yaw_angle(launch_origin, candidate->target_position);
        }

        const auto forward = Trait::calc_view_basis(pitch, yaw).forward;
        const auto distance = launcher.eye_origin.distance_to(candidate->target_position);

        return AimSolution<float>{
                .angles = {pitch, yaw},
                .aim_point = launcher.eye_origin + forward * distance,
                .predicted_target_position = candidate->target_position,
                .time_of_flight = candidate->time,
        };
#else
        throw std::runtime_error(
                std::format("{} AVX2 feature is not enabled!", std::source_location::current().function_name()));
#endif
    }

    std::optional<ProjPredEngineAvx2::Candidate> ProjPredEngineAvx2::find_candidate(
            [[maybe_unused]] const Vector3<float>& launch_origin, [[maybe_unused]] const Target<float>& target,
            [[maybe_unused]] const float bullet_gravity, [[maybe_unused]] const float launch_speed) const
    {
#ifdef OMATH_PROJ_PRED_AVX2_AVAILABLE
        constexpr int simd_factor = 8;
        const float step = m_simulation_time_step;

        // Speed the projectile would need to be at the predicted target exactly at each of eight candidate times.
        // Times where that is at most the launch speed get a scalar solve.
        for (std::size_t block = 0;; ++block)
        {
            const float block_time = step + step * static_cast<float>(simd_factor) * static_cast<float>(block);
            if (block_time > m_maximum_simulation_time)
                break;

            const __m256 times = _mm256_setr_ps(block_time, block_time + step, block_time + step * 2,
                                                block_time + step * 3, block_time + step * 4, block_time + step * 5,
                                                block_time + step * 6, block_time + step * 7);

            const __m256 target_x =
                    _mm256_fmadd_ps(_mm256_set1_ps(target.m_velocity.x), times, _mm256_set1_ps(target.m_origin.x));
            const __m256 target_y =
                    _mm256_fmadd_ps(_mm256_set1_ps(target.m_velocity.y), times, _mm256_set1_ps(target.m_origin.y));
            const __m256 times_sq = _mm256_mul_ps(times, times);
            const __m256 target_z = _mm256_fmadd_ps(
                    _mm256_set1_ps(target.m_velocity.z), times,
                    _mm256_fnmadd_ps(_mm256_set1_ps(target.m_is_airborne ? 0.5f * m_gravity_constant : 0.f), times_sq,
                                     _mm256_set1_ps(target.m_origin.z)));

            const __m256 delta_x = _mm256_sub_ps(target_x, _mm256_set1_ps(launch_origin.x));
            const __m256 delta_y = _mm256_sub_ps(target_y, _mm256_set1_ps(launch_origin.y));
            const __m256 delta_z = _mm256_sub_ps(target_z, _mm256_set1_ps(launch_origin.z));

            const __m256 d_sqr = _mm256_add_ps(_mm256_mul_ps(delta_x, delta_x), _mm256_mul_ps(delta_y, delta_y));

            const __m256 bg_times_sq = _mm256_mul_ps(_mm256_set1_ps(bullet_gravity), times_sq);
            const __m256 term = _mm256_add_ps(delta_z, _mm256_mul_ps(_mm256_set1_ps(0.5f), bg_times_sq));
            const __m256 term_sq = _mm256_mul_ps(term, term);
            const __m256 numerator = _mm256_add_ps(d_sqr, term_sq);
            const __m256 denominator = _mm256_add_ps(times_sq, _mm256_set1_ps(1e-8f)); // Avoid division by zero
            const __m256 required_speed_sqr = _mm256_div_ps(numerator, denominator);

            const __m256 launch_speed_sqr = _mm256_set1_ps(launch_speed * launch_speed + 1e-3f);
            const __m256 mask = _mm256_cmp_ps(required_speed_sqr, launch_speed_sqr, _CMP_LE_OQ);

            const auto valid_mask = static_cast<unsigned>(_mm256_movemask_ps(mask));
            if (!valid_mask)
                continue;

            alignas(32) float candidate_times[simd_factor];
            _mm256_store_ps(candidate_times, times);

            for (int i = 0; i < simd_factor; ++i)
            {
                if (!(valid_mask & (1u << i)))
                    continue;

                const float candidate_time = candidate_times[i];
                if (candidate_time > m_maximum_simulation_time)
                    continue;

                // Fine search around the candidate time
                for (int offset = -2; offset <= 2; ++offset)
                {
                    const float fine_time = candidate_time + step * static_cast<float>(offset);
                    if (fine_time < 0.f)
                        continue;

                    const auto target_pos = source_engine::PredEngineTrait::predict_target_position(target, fine_time,
                                                                                                    m_gravity_constant);

                    const auto pitch =
                            calculate_pitch(launch_origin, target_pos, bullet_gravity, launch_speed, fine_time);
                    if (!pitch)
                        continue;

                    return Candidate{fine_time, target_pos, *pitch};
                }
            }
        }

        return std::nullopt;
#else
        throw std::runtime_error(
                std::format("{} AVX2 feature is not enabled!", std::source_location::current().function_name()));
#endif
    }

    std::optional<float> ProjPredEngineAvx2::calculate_pitch([[maybe_unused]] const Vector3<float>& launch_origin,
                                                             [[maybe_unused]] const Vector3<float>& target_pos,
                                                             [[maybe_unused]] const float bullet_gravity,
                                                             [[maybe_unused]] const float launch_speed,
                                                             [[maybe_unused]] const float time)
    {
#ifdef OMATH_PROJ_PRED_AVX2_AVAILABLE
        if (time <= 0.0f)
            return std::nullopt;

        const Vector3 delta = target_pos - launch_origin;
        const float d_sqr = delta.x * delta.x + delta.y * delta.y;
        const float h = delta.z;

        const float term = h + 0.5f * bullet_gravity * time * time;
        const float required_speed_sqr = (d_sqr + term * term) / (time * time);

        if (required_speed_sqr > launch_speed * launch_speed + 1e-3f)
            return std::nullopt;

        if (d_sqr == 0.0f)
            return term >= 0.0f ? 90.0f : -90.0f;

        const float d = std::sqrt(d_sqr);
        return angles::radians_to_degrees(std::atan(term / d));
#else
        throw std::runtime_error(
                std::format("{} AVX2 feature is not enabled!", std::source_location::current().function_name()));
#endif
    }
} // namespace omath::projectile_prediction
