//
// Created by Vladislav on 19.03.2026.
//

#pragma once
#include "omath/linear_algebra/vector3.hpp"
#include <functional>
#include <iterator>
#include <optional>

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

            const Vector2<float> current_angles_vec = {current_target_angles.pitch.as_degrees(),
                                                       current_target_angles.yaw.as_degrees()};
            const Vector2<float> best_angles_vec = {best_target_angles.pitch.as_degrees(),
                                                    best_target_angles.yaw.as_degrees()};

            const auto current_target_distance = camera_angles_vec.distance_to(current_angles_vec);
            const auto best_target_distance = camera_angles_vec.distance_to(best_angles_vec);
            if (current_target_distance < best_target_distance)
                best_target = current;
        }
        return best_target;
    }

} // namespace omath::algorithm