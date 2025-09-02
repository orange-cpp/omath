//
// Created by Vlad on 9/3/2025.
//
module;
#include <optional>
#include <source_location>

#if defined(OMATH_USE_AVX2) && defined(__i386__) && defined(__x86_64__)
#include <immintrin.h>
#else
#include <format>
#endif

export module omath.projectile_prediction.proj_pred_engine_avx2;

import omath.projectile_prediction.proj_pred_engine;
import omath.projectile_prediction.projectile;
import omath.projectile_prediction.target;

namespace omath::projectile_prediction
{
    class ProjPredEngineAvx2 final : public ProjPredEngineInterface
    {
    public:
        [[nodiscard]] std::optional<Vector3<float>>
        maybe_calculate_aim_point([[maybe_unused]] const Projectile& projectile,
                                  [[maybe_unused]] const Target& target) const override
        {
#if defined(OMATH_USE_AVX2) && defined(__i386__) && defined(__x86_64__)
            const float bulletGravity = m_gravityConstant * projectile.m_gravityScale;
            const float v0 = projectile.m_launchSpeed;
            const float v0Sqr = v0 * v0;
            const Vector3 projOrigin = projectile.m_origin;

            constexpr int SIMD_FACTOR = 8;
            float currentTime = m_simulationTimeStep;

            for (; currentTime <= m_maximumSimulationTime; currentTime += m_simulationTimeStep * SIMD_FACTOR)
            {
                const __m256 times =
                        _mm256_setr_ps(currentTime, currentTime + m_simulationTimeStep,
                                       currentTime + m_simulationTimeStep * 2, currentTime + m_simulationTimeStep * 3,
                                       currentTime + m_simulationTimeStep * 4, currentTime + m_simulationTimeStep * 5,
                                       currentTime + m_simulationTimeStep * 6, currentTime + m_simulationTimeStep * 7);

                const __m256 targetX =
                        _mm256_fmadd_ps(_mm256_set1_ps(target.m_velocity.x), times, _mm256_set1_ps(target.m_origin.x));
                const __m256 targetY =
                        _mm256_fmadd_ps(_mm256_set1_ps(target.m_velocity.y), times, _mm256_set1_ps(target.m_origin.y));
                const __m256 timesSq = _mm256_mul_ps(times, times);
                const __m256 targetZ = _mm256_fmadd_ps(_mm256_set1_ps(target.m_velocity.z), times,
                                                       _mm256_fnmadd_ps(_mm256_set1_ps(0.5f * m_gravityConstant),
                                                                        timesSq, _mm256_set1_ps(target.m_origin.z)));

                const __m256 deltaX = _mm256_sub_ps(targetX, _mm256_set1_ps(projOrigin.x));
                const __m256 deltaY = _mm256_sub_ps(targetY, _mm256_set1_ps(projOrigin.y));
                const __m256 deltaZ = _mm256_sub_ps(targetZ, _mm256_set1_ps(projOrigin.z));

                const __m256 dSqr = _mm256_add_ps(_mm256_mul_ps(deltaX, deltaX), _mm256_mul_ps(deltaY, deltaY));

                const __m256 bgTimesSq = _mm256_mul_ps(_mm256_set1_ps(bulletGravity), timesSq);
                const __m256 term = _mm256_add_ps(deltaZ, _mm256_mul_ps(_mm256_set1_ps(0.5f), bgTimesSq));
                const __m256 termSq = _mm256_mul_ps(term, term);
                const __m256 numerator = _mm256_add_ps(dSqr, termSq);
                const __m256 denominator = _mm256_add_ps(timesSq, _mm256_set1_ps(1e-8f)); // Avoid division by zero
                const __m256 requiredV0Sqr = _mm256_div_ps(numerator, denominator);

                const __m256 v0SqrVec = _mm256_set1_ps(v0Sqr + 1e-3f);
                const __m256 mask = _mm256_cmp_ps(requiredV0Sqr, v0SqrVec, _CMP_LE_OQ);

                const unsigned validMask = _mm256_movemask_ps(mask);

                if (!validMask)
                    continue;

                alignas(32) float validTimes[SIMD_FACTOR];
                _mm256_store_ps(validTimes, times);

                for (int i = 0; i < SIMD_FACTOR; ++i)
                {
                    if (!(validMask & (1 << i)))
                        continue;

                    const float candidateTime = validTimes[i];

                    if (candidateTime > m_maximumSimulationTime)
                        continue;

                    // Fine search around candidate time
                    for (float fineTime = candidateTime - m_simulationTimeStep * 2;
                         fineTime <= candidateTime + m_simulationTimeStep * 2; fineTime += m_simulationTimeStep)
                    {
                        if (fineTime < 0)
                            continue;

                        const Vector3 targetPos = target.PredictPosition(fineTime, m_gravityConstant);
                        const auto pitch = CalculatePitch(projOrigin, targetPos, bulletGravity, v0, fineTime);
                        if (!pitch)
                            continue;

                        const Vector3 delta = targetPos - projOrigin;
                        const float d = std::sqrt(delta.x * delta.x + delta.y * delta.y);
                        const float height = d * std::tan(angles::DegreesToRadians(*pitch));
                        return Vector3(targetPos.x, targetPos.y, projOrigin.z + height);
                    }
                }
            }

            // Fallback scalar processing for remaining times
            for (; currentTime <= m_maximumSimulationTime; currentTime += m_simulationTimeStep)
            {
                const Vector3 targetPos = target.PredictPosition(currentTime, m_gravityConstant);
                const auto pitch = CalculatePitch(projOrigin, targetPos, bulletGravity, v0, currentTime);
                if (!pitch)
                    continue;

                const Vector3 delta = targetPos - projOrigin;
                const float d = std::sqrt(delta.x * delta.x + delta.y * delta.y);
                const float height = d * std::tan(angles::DegreesToRadians(*pitch));
                return Vector3(targetPos.x, targetPos.y, projOrigin.z + height);
            }

            return std::nullopt;
#else
            throw std::runtime_error(
                    std::format("{} AVX2 feature is not enabled!", std::source_location::current().function_name()));
#endif
        }

        ProjPredEngineAvx2(const float gravity_constant, const float simulation_time_step,
                           const float maximum_simulation_time)
            : m_gravity_constant(gravity_constant), m_simulation_time_step(simulation_time_step),
              m_maximum_simulation_time(maximum_simulation_time)
        {
        }
        ~ProjPredEngineAvx2() override = default;

    private:
        [[nodiscard]] static std::optional<float> calculate_pitch([[maybe_unused]] const Vector3<float>& proj_origin,
                                                                  [[maybe_unused]] const Vector3<float>& target_pos,
                                                                  [[maybe_unused]] float bullet_gravity,
                                                                  [[maybe_unused]] float v0,
                                                                  [[maybe_unused]] float time)
        {
#if defined(OMATH_USE_AVX2) && defined(__i386__) && defined(__x86_64__)
            if (time <= 0.0f)
                return std::nullopt;

            const Vector3 delta = target_pos - proj_origin;
            const float dSqr = delta.x * delta.x + delta.y * delta.y;
            const float h = delta.z;

            const float term = h + 0.5f * bullet_gravity * time * time;
            const float requiredV0Sqr = (dSqr + term * term) / (time * time);
            const float v0Sqr = v0 * v0;

            if (requiredV0Sqr > v0Sqr + 1e-3f)
                return std::nullopt;

            if (dSqr == 0.0f)
                return term >= 0.0f ? 90.0f : -90.0f;

            const float d = std::sqrt(dSqr);
            const float tanTheta = term / d;
            return angles::RadiansToDegrees(std::atan(tanTheta));
#else
            throw std::runtime_error(
                    std::format("{} AVX2 feature is not enabled!", std::source_location::current().function_name()));
#endif
        }

        // We use [[maybe_unused]] here since AVX2 is not available for ARM and ARM64 CPU
        [[maybe_unused]] const float m_gravity_constant;
        [[maybe_unused]] const float m_simulation_time_step;
        [[maybe_unused]] const float m_maximum_simulation_time;
    };
} // namespace omath::projectile_prediction