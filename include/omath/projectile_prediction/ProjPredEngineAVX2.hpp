//
// Created by Vlad on 2/23/2025.
//
#pragma once
#include "ProjPredEngine.hpp"

namespace omath::projectile_prediction
{
    class ProjPredEngineAVX2 final : public ProjPredEngine
    {
    public:
        [[nodiscard]] std::optional<Vector3<float>> MaybeCalculateAimPoint(const Projectile& projectile,
                                                                    const Target& target) const override;


        ProjPredEngineAVX2(float gravityConstant, float simulationTimeStep, float maximumSimulationTime);
        ~ProjPredEngineAVX2() override = default;

    private:
        [[nodiscard]] static std::optional<float> CalculatePitch(const Vector3<float>& projOrigin, const Vector3<float>& targetPos,
                                                                 float bulletGravity, float v0, float time);
        const float m_gravityConstant;
        const float m_simulationTimeStep;
        const float m_maximumSimulationTime;
    };
} // namespace omath::projectile_prediction
