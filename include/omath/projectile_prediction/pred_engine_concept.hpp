//
// Created by Vlad on 9/19/2026.
//

#pragma once
#include "omath/linear_algebra/vector3.hpp"
#include "omath/projectile_prediction/launcher.hpp"
#include "omath/projectile_prediction/projectile.hpp"
#include "omath/projectile_prediction/target.hpp"
#include <concepts>

namespace omath::projectile_prediction
{
    template<class T, class ArithmeticType>
    concept PredEngineConcept =
            requires(const Projectile<ArithmeticType>& projectile, const Target<ArithmeticType>& target,
                     const Vector3<ArithmeticType>& vec_a, const Vector3<ArithmeticType>& vec_b, ArithmeticType pitch,
                     ArithmeticType yaw, ArithmeticType time, ArithmeticType gravity) {
                {
                    T::predict_projectile_position(vec_a, projectile, pitch, yaw, time, gravity)
                } -> std::same_as<Vector3<ArithmeticType>>;
                { T::predict_target_position(target, time, gravity) } -> std::same_as<Vector3<ArithmeticType>>;
                { T::calc_vector_2d_distance(vec_a) } -> std::same_as<ArithmeticType>;
                { T::get_vector_height_coordinate(vec_b) } -> std::same_as<ArithmeticType>;
                { T::calc_view_basis(pitch, yaw) } -> std::same_as<ViewBasis<ArithmeticType>>;
                { T::calc_direct_pitch_angle(vec_a, vec_b) } -> std::same_as<ArithmeticType>;
                { T::calc_direct_yaw_angle(vec_a, vec_b) } -> std::same_as<ArithmeticType>;

                requires noexcept(T::predict_projectile_position(vec_a, projectile, pitch, yaw, time, gravity));
                requires noexcept(T::predict_target_position(target, time, gravity));
                requires noexcept(T::calc_vector_2d_distance(vec_a));
                requires noexcept(T::get_vector_height_coordinate(vec_b));
                requires noexcept(T::calc_view_basis(pitch, yaw));
                requires noexcept(T::calc_direct_pitch_angle(vec_a, vec_b));
                requires noexcept(T::calc_direct_yaw_angle(vec_a, vec_b));
            };
} // namespace omath::projectile_prediction
