//
// Created by Vladislav on 19.03.2026.
//

#pragma once
#include "omath/linear_algebra/vector3.hpp"
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>

namespace omath::algorithm
{
    template<class CameraType, std::input_or_output_iterator IteratorType, class FilterT>
    requires std::is_invocable_r_v<bool, std::function<FilterT>, std::iter_reference_t<IteratorType>>
    [[nodiscard]]
    IteratorType get_closest_target_by_fov(const IteratorType& begin, const IteratorType& end, const CameraType& camera,
                                           auto get_position,
                                           const std::optional<std::function<FilterT>>& filter_func = std::nullopt)
    {
        auto best_target = end;
        const auto& camera_angles = camera.get_view_angles();
        const Vector2<float> camera_angles_vec = {camera_angles.pitch.as_degrees(), camera_angles.yaw.as_degrees()};

        for (auto current = begin; current != end; current = std::next(current))
        {
            if (filter_func && !filter_func.value()(*current))
                continue;

            if (best_target == end)
            {
                best_target = current;
                continue;
            }
            const auto current_target_angles = camera.calc_look_at_angles(get_position(*current));
            const auto best_target_angles = camera.calc_look_at_angles(get_position(*best_target));

            const auto current_target_distance = camera_angles_vec.distance_to(current_target_angles.as_vector3());
            const auto best_target_distance = camera_angles.as_vector3().distance_to(best_target_angles.as_vector3());
            if (current_target_distance < best_target_distance)
                best_target = current;
        }
        return best_target;
    }

    template<class CameraType, std::ranges::range RangeType, class FilterT>
    requires std::is_invocable_r_v<bool, std::function<FilterT>,
                                   std::ranges::range_reference_t<const RangeType>>
    [[nodiscard]]
    auto get_closest_target_by_fov(const RangeType& range, const CameraType& camera,
                                   auto get_position,
                                   const std::optional<std::function<FilterT>>& filter_func = std::nullopt)
    {
        return get_closest_target_by_fov<CameraType, decltype(std::ranges::begin(range)), FilterT>(
                std::ranges::begin(range), std::ranges::end(range), camera, get_position, filter_func);
    }

    // ── By world-space distance ───────────────────────────────────────────────

    template<std::input_or_output_iterator IteratorType, class FilterT>
    requires std::is_invocable_r_v<bool, std::function<FilterT>, std::iter_reference_t<IteratorType>>
    [[nodiscard]]
    IteratorType get_closest_target_by_distance(const IteratorType& begin, const IteratorType& end,
                                                const Vector3<float>& origin, auto get_position,
                                                const std::optional<std::function<FilterT>>& filter_func = std::nullopt)
    {
        auto best_target = end;

        for (auto current = begin; current != end; current = std::next(current))
        {
            if (filter_func && !filter_func.value()(*current))
                continue;

            if (best_target == end)
            {
                best_target = current;
                continue;
            }

            if (origin.distance_to(get_position(*current)) < origin.distance_to(get_position(*best_target)))
                best_target = current;
        }
        return best_target;
    }

    template<std::ranges::range RangeType, class FilterT>
    requires std::is_invocable_r_v<bool, std::function<FilterT>,
                                   std::ranges::range_reference_t<const RangeType>>
    [[nodiscard]]
    auto get_closest_target_by_distance(const RangeType& range, const Vector3<float>& origin,
                                        auto get_position,
                                        const std::optional<std::function<FilterT>>& filter_func = std::nullopt)
    {
        return get_closest_target_by_distance<decltype(std::ranges::begin(range)), FilterT>(
                std::ranges::begin(range), std::ranges::end(range), origin, get_position, filter_func);
    }

} // namespace omath::algorithm