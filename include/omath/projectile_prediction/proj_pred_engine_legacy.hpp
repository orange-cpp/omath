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
    // ReSharper disable once CppClassCanBeFinal
    class ProjPredEngineLegacy : public ProjPredEngineInterface
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

        // Realization of this formula:
        // https://stackoverflow.com/questions/54917375/how-to-calculate-the-angle-to-shoot-a-bullet-in-order-to-hit-a-moving-target
        /*
        \[
        \theta \;=\; \arctan\!\Biggl(
        \frac{%
        v^{2}\;\pm\;\sqrt{\,v^{4}-g\!\left(gx^{2}+2yv^{2}\right)\,}
        }{%
        gx
       }\Biggr)
       \]
        */
        [[nodiscard]]
        std::optional<float>
        maybe_calculate_projectile_launch_pitch_angle(const Projectile& projectile,
                                                      const Vector3<float>& target_position) const noexcept;

        [[nodiscard]]
        bool is_projectile_reached_target(const Vector3<float>& target_position, const Projectile& projectile,
                                          float pitch, float time) const noexcept;

    protected:
        // NOTE: Override this if you need to use engine with different coordinate system
        // Like where Z is not height coordinate
        // ===============================================================================================
        [[nodiscard]]
        virtual float calc_vector_2d_distance(const Vector3<float>& delta) const;

        [[nodiscard]]
        virtual float get_vector_height_coordinate(const Vector3<float>& vec) const;

        [[nodiscard]]
        virtual Vector3<float> calc_viewpoint_from_angles(const Projectile& projectile,
                                                          Vector3<float> predicted_target_position,
                                                          std::optional<float> projectile_pitch) const;

        [[nodiscard]]
        virtual Vector3<float> predict_projectile_position(const Projectile& projectile, float pitch, float yaw,
                                                           float time, float gravity) const;
        // ===============================================================================================
    };
} // namespace omath::projectile_prediction
