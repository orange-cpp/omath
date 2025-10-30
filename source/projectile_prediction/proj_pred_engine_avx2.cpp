//
// Created by Vlad on 2/23/2025.
//
#include "omath/projectile_prediction/proj_pred_engine_avx2.hpp"
#include <source_location>
#include <stdexcept>

#if defined(OMATH_USE_AVX2) && defined(__i386__) && defined(__x86_64__)
#include <immintrin.h>
#else
#include <format>
#endif

namespace omath::projectile_prediction
{
    std::optional<Vector3<float>>
    ProjPredEngineAvx2::maybe_calculate_aim_point([[maybe_unused]] const Projectile& projectile,
                                                  [[maybe_unused]] const Target& target) const
    {
#if defined(OMATH_USE_AVX2) && defined(__i386__) && defined(__x86_64__)
        const float bullet_gravity = m_gravity_constant * projectile.m_gravity_scale;
        const float v0 = projectile.m_launch_speed;
        const float v0_sqr = v0 * v0;
        const Vector3 proj_origin = projectile.m_origin;

        constexpr int SIMD_FACTOR = 8;
        float current_time = m_simulation_time_step;

        for (; current_time <= m_maximum_simulation_time; current_time += m_simulation_time_step * SIMD_FACTOR)
        {
            const __m256 times
                    = _mm256_setr_ps(current_time, current_time + m_simulation_time_step,
                                     current_time + m_simulation_time_step * 2, current_time + m_simulation_time_step * 3,
                                     current_time + m_simulation_time_step * 4, current_time + m_simulation_time_step * 5,
                                     current_time + m_simulation_time_step * 6, current_time + m_simulation_time_step * 7);

            const __m256 target_x
                    = _mm256_fmadd_ps(_mm256_set1_ps(target.m_velocity.x), times, _mm256_set1_ps(target.m_origin.x));
            const __m256 target_y
                    = _mm256_fmadd_ps(_mm256_set1_ps(target.m_velocity.y), times, _mm256_set1_ps(target.m_origin.y));
            const __m256 times_sq = _mm256_mul_ps(times, times);
            const __m256 target_z = _mm256_fmadd_ps(_mm256_set1_ps(target.m_velocity.z), times,
                                                   _mm256_fnmadd_ps(_mm256_set1_ps(0.5f * m_gravity_constant), times_sq,
                                                                    _mm256_set1_ps(target.m_origin.z)));

            const __m256 delta_x = _mm256_sub_ps(target_x, _mm256_set1_ps(proj_origin.x));
            const __m256 delta_y = _mm256_sub_ps(target_y, _mm256_set1_ps(proj_origin.y));
            const __m256 delta_z = _mm256_sub_ps(target_z, _mm256_set1_ps(proj_origin.z));

            const __m256 d_sqr = _mm256_add_ps(_mm256_mul_ps(delta_x, delta_x), _mm256_mul_ps(delta_y, delta_y));

            const __m256 bg_times_sq = _mm256_mul_ps(_mm256_set1_ps(bullet_gravity), times_sq);
            const __m256 term = _mm256_add_ps(delta_z, _mm256_mul_ps(_mm256_set1_ps(0.5f), bg_times_sq));
            const __m256 term_sq = _mm256_mul_ps(term, term);
            const __m256 numerator = _mm256_add_ps(d_sqr, term_sq);
            const __m256 denominator = _mm256_add_ps(times_sq, _mm256_set1_ps(1e-8f)); // Avoid division by zero
            const __m256 required_v0_sqr = _mm256_div_ps(numerator, denominator);

            const __m256 v0_sqr_vec = _mm256_set1_ps(v0_sqr + 1e-3f);
            const __m256 mask = _mm256_cmp_ps(required_v0_sqr, v0_sqr_vec, _CMP_LE_OQ);

            const unsigned valid_mask = _mm256_movemask_ps(mask);

            if (!valid_mask)
                continue;

            alignas(32) float valid_times[SIMD_FACTOR];
            _mm256_store_ps(valid_times, times);

            for (int i = 0; i < SIMD_FACTOR; ++i)
            {
                if (!(valid_mask & (1 << i)))
                    continue;

                const float candidate_time = valid_times[i];

                if (candidate_time > m_maximum_simulation_time)
                    continue;

                // Fine search around candidate time
                for (float fine_time = candidate_time - m_simulation_time_step * 2;
                     fine_time <= candidate_time + m_simulation_time_step * 2; fine_time += m_simulation_time_step)
                {
                    if (fine_time < 0)
                        continue;

                    // Manually compute predicted target position to avoid adding method to Target
                    Vector3 target_pos = target.m_origin + target.m_velocity * fine_time;
                    if (target.m_is_airborne)
                        target_pos.z -= 0.5f * m_gravity_constant * fine_time * fine_time;

                    const auto pitch = calculate_pitch(proj_origin, target_pos, bullet_gravity, v0, fine_time);
                    if (!pitch)
                        continue;

                    const Vector3 delta = target_pos - proj_origin;
                    const float d = std::sqrt(delta.x * delta.x + delta.y * delta.y);
                    const float height = d * std::tan(angles::degrees_to_radians(*pitch));
                    return Vector3(target_pos.x, target_pos.y, proj_origin.z + height);
                }
            }
        }

        // Fallback scalar processing for remaining times
        for (; current_time <= m_maximum_simulation_time; current_time += m_simulation_time_step)
        {
            Vector3 target_pos = target.m_origin + target.m_velocity * current_time;
            if (target.m_is_airborne)
                target_pos.z -= 0.5f * m_gravity_constant * current_time * current_time;

            const auto pitch = calculate_pitch(proj_origin, target_pos, bullet_gravity, v0, current_time);
            if (!pitch)
                continue;

            const Vector3 delta = target_pos - proj_origin;
            const float d = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            const float height = d * std::tan(angles::degrees_to_radians(*pitch));
            return Vector3(target_pos.x, target_pos.y, proj_origin.z + height);
        }

        return std::nullopt;
#else
        throw std::runtime_error(
                std::format("{} AVX2 feature is not enabled!", std::source_location::current().function_name()));
#endif
    }
    ProjPredEngineAvx2::ProjPredEngineAvx2(const float gravity_constant, const float simulation_time_step,
                                           const float maximum_simulation_time)
        : m_gravity_constant(gravity_constant), m_simulation_time_step(simulation_time_step),
          m_maximum_simulation_time(maximum_simulation_time)
    {
    }
    std::optional<float> ProjPredEngineAvx2::calculate_pitch([[maybe_unused]] const Vector3<float>& proj_origin,
                                                             [[maybe_unused]] const Vector3<float>& target_pos,
                                                             [[maybe_unused]] const float bullet_gravity,
                                                             [[maybe_unused]] const float v0,
                                                             [[maybe_unused]] const float time)
    {
#if defined(OMATH_USE_AVX2) && defined(__i386__) && defined(__x86_64__)
        if (time <= 0.0f)
            return std::nullopt;

        const Vector3 delta = target_pos - proj_origin;
        const float d_sqr = delta.x * delta.x + delta.y * delta.y;
        const float h = delta.z;

        const float term = h + 0.5f * bullet_gravity * time * time;
        const float required_v0_sqr = (d_sqr + term * term) / (time * time);
        const float v0_sqr = v0 * v0;

        if (required_v0_sqr > v0_sqr + 1e-3f)
            return std::nullopt;

        if (d_sqr == 0.0f)
            return term >= 0.0f ? 90.0f : -90.0f;

        const float d = std::sqrt(d_sqr);
        const float tan_theta = term / d;
        return angles::radians_to_degrees(std::atan(tan_theta));
#else
        throw std::runtime_error(
                std::format("{} AVX2 feature is not enabled!", std::source_location::current().function_name()));
#endif
    }
} // namespace omath::projectile_prediction
