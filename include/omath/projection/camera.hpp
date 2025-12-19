//
// Created by Vlad on 27.08.2024.
//

#pragma once

#include "omath/linear_algebra/mat.hpp"
#include "omath/linear_algebra/triangle.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include "omath/projection/error_codes.hpp"
#include <cmath>
#include <expected>
#include <omath/trigonometry/angle.hpp>
#include <type_traits>

#ifdef OMATH_BUILD_TESTS
// ReSharper disable CppInconsistentNaming
class UnitTestProjection_Projection_Test;
class UnitTestProjection_ScreenToNdcTopLeft_Test;
class UnitTestProjection_ScreenToNdcBottomLeft_Test;
// ReSharper restore CppInconsistentNaming

#endif

namespace omath::projection
{
    class ViewPort final
    {
    public:
        float m_width;
        float m_height;

        [[nodiscard]] constexpr float aspect_ratio() const
        {
            return m_width / m_height;
        }
    };
    using FieldOfView = Angle<float, 0.f, 180.f, AngleFlags::Clamped>;

    template<class T, class MatType, class ViewAnglesType>
    concept CameraEngineConcept =
            requires(const Vector3<float>& cam_origin, const Vector3<float>& look_at, const ViewAnglesType& angles,
                     const FieldOfView& fov, const ViewPort& viewport, float znear, float zfar) {
                // Presence + return types
                { T::calc_look_at_angle(cam_origin, look_at) } -> std::same_as<ViewAnglesType>;
                { T::calc_view_matrix(angles, cam_origin) } -> std::same_as<MatType>;
                { T::calc_projection_matrix(fov, viewport, znear, zfar) } -> std::same_as<MatType>;

                // Enforce noexcept as in the trait declaration
                requires noexcept(T::calc_look_at_angle(cam_origin, look_at));
                requires noexcept(T::calc_view_matrix(angles, cam_origin));
                requires noexcept(T::calc_projection_matrix(fov, viewport, znear, zfar));
            };

    template<class Mat4X4Type, class ViewAnglesType, class TraitClass>
    requires CameraEngineConcept<TraitClass, Mat4X4Type, ViewAnglesType>
    class Camera final
    {
#ifdef OMATH_BUILD_TESTS
        friend UnitTestProjection_Projection_Test;
        friend UnitTestProjection_ScreenToNdcTopLeft_Test;
        friend UnitTestProjection_ScreenToNdcBottomLeft_Test;
#endif
    public:
        enum class ScreenStart
        {
            TOP_LEFT_CORNER,
            BOTTOM_LEFT_CORNER,
        };

        ~Camera() = default;
        Camera(const Vector3<float>& position, const ViewAnglesType& view_angles, const ViewPort& view_port,
               const FieldOfView& fov, const float near, const float far) noexcept
            : m_view_port(view_port), m_field_of_view(fov), m_far_plane_distance(far), m_near_plane_distance(near),
              m_view_angles(view_angles), m_origin(position)
        {
        }

        void look_at(const Vector3<float>& target)
        {
            m_view_angles = TraitClass::calc_look_at_angle(m_origin, target);
            m_view_projection_matrix = std::nullopt;
        }

    protected:
        [[nodiscard]] Mat4X4Type calc_view_projection_matrix() const noexcept
        {
            return TraitClass::calc_projection_matrix(m_field_of_view, m_view_port, m_near_plane_distance,
                                                      m_far_plane_distance)
                   * TraitClass::calc_view_matrix(m_view_angles, m_origin);
        }

    public:
        [[nodiscard]] const Mat4X4Type& get_view_projection_matrix() const noexcept
        {
            if (!m_view_projection_matrix.has_value())
                m_view_projection_matrix = calc_view_projection_matrix();

            return m_view_projection_matrix.value();
        }

        void set_field_of_view(const FieldOfView& fov) noexcept
        {
            m_field_of_view = fov;
            m_view_projection_matrix = std::nullopt;
        }

        void set_near_plane(const float near) noexcept
        {
            m_near_plane_distance = near;
            m_view_projection_matrix = std::nullopt;
        }

        void set_far_plane(const float far) noexcept
        {
            m_far_plane_distance = far;
            m_view_projection_matrix = std::nullopt;
        }

        void set_view_angles(const ViewAnglesType& view_angles) noexcept
        {
            m_view_angles = view_angles;
            m_view_projection_matrix = std::nullopt;
        }

        void set_origin(const Vector3<float>& origin) noexcept
        {
            m_origin = origin;
            m_view_projection_matrix = std::nullopt;
        }

        void set_view_port(const ViewPort& view_port) noexcept
        {
            m_view_port = view_port;
            m_view_projection_matrix = std::nullopt;
        }

        [[nodiscard]] const FieldOfView& get_field_of_view() const noexcept
        {
            return m_field_of_view;
        }

        [[nodiscard]] const float& get_near_plane() const noexcept
        {
            return m_near_plane_distance;
        }

        [[nodiscard]] const float& get_far_plane() const noexcept
        {
            return m_far_plane_distance;
        }

        [[nodiscard]] const ViewAnglesType& get_view_angles() const noexcept
        {
            return m_view_angles;
        }

        [[nodiscard]] const Vector3<float>& get_origin() const noexcept
        {
            return m_origin;
        }

        template<ScreenStart screen_start = ScreenStart::TOP_LEFT_CORNER>
        [[nodiscard]] std::expected<Vector3<float>, Error>
        world_to_screen(const Vector3<float>& world_position) const noexcept
        {
            const auto normalized_cords = world_to_view_port(world_position);

            if (!normalized_cords.has_value())
                return std::unexpected{normalized_cords.error()};

            if constexpr (screen_start == ScreenStart::TOP_LEFT_CORNER)
                return ndc_to_screen_position_from_top_left_corner(*normalized_cords);
            else if constexpr (screen_start == ScreenStart::BOTTOM_LEFT_CORNER)
                return ndc_to_screen_position_from_bottom_left_corner(*normalized_cords);
            else
                std::unreachable();
        }

        [[nodiscard]] bool is_culled_by_frustum(const Triangle<Vector3<float>>& triangle) const noexcept
        {
            // Transform to clip space (before perspective divide)
            auto to_clip = [this](const Vector3<float>& point)
            {
                auto clip = get_view_projection_matrix()
                            * mat_column_from_vector<float, Mat4X4Type::get_store_ordering()>(point);
                return std::array<float, 4>{
                        clip.at(0, 0), // x
                        clip.at(1, 0), // y
                        clip.at(2, 0), // z
                        clip.at(3, 0) // w
                };
            };

            const auto c0 = to_clip(triangle.m_vertex1);
            const auto c1 = to_clip(triangle.m_vertex2);
            const auto c2 = to_clip(triangle.m_vertex3);

            // If all vertices are behind the camera (w <= 0), trivially reject
            if (c0[3] <= 0.f && c1[3] <= 0.f && c2[3] <= 0.f)
                return true;

            // Helper: all three vertices outside the same clip plane
            auto all_outside_plane = [](const int axis, const std::array<float, 4>& a, const std::array<float, 4>& b,
                                        const std::array<float, 4>& c, const bool positive_side)
            {
                if (positive_side)
                    return a[axis] > a[3] && b[axis] > b[3] && c[axis] > c[3];
                return a[axis] < -a[3] && b[axis] < -b[3] && c[axis] < -c[3];
            };

            // Clip volume in clip space (OpenGL-style):
            // -w <= x <= w
            // -w <= y <= w
            // -w <= z <= w

            for (int i = 0; i < 3; i++)
            {
                if (all_outside_plane(i, c0, c1, c2, false))
                    return true; // x < -w (left)
                if (all_outside_plane(i, c0, c1, c2, true))
                    return true; // x >  w (right)
            }
            return false;
        }

        [[nodiscard]] std::expected<Vector3<float>, Error>
        world_to_view_port(const Vector3<float>& world_position) const noexcept
        {
            auto projected = get_view_projection_matrix()
                             * mat_column_from_vector<float, Mat4X4Type::get_store_ordering()>(world_position);

            constexpr float w_epsilon = 1e-6f;
            const auto w = projected.at(3, 0);
            if (w <= w_epsilon) {
                return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);
            }

            projected /= w;

            if (is_ndc_out_of_bounds(projected)) {
                return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);
            }

            return Vector3<float>{projected.at(0, 0), projected.at(1, 0), projected.at(2, 0)};
        }
        [[nodiscard]]
        std::expected<Vector3<float>, Error> view_port_to_screen(const Vector3<float>& ndc) const noexcept
        {
            const auto inv_view_proj = get_view_projection_matrix().inverted();

            if (!inv_view_proj) {
                return std::unexpected(Error::INV_VIEW_PROJ_MAT_DET_EQ_ZERO);
            }

            auto inverted_projection =
                    inv_view_proj.value() * mat_column_from_vector<float, Mat4X4Type::get_store_ordering()>(ndc);

            constexpr float w_epsilon = 1e-6f;
            const auto w = inverted_projection.at(3, 0);
            if (std::abs(w) < w_epsilon) {
                return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);
            }

            inverted_projection /= w;

            const Vector3<float> world_pos{inverted_projection.at(0, 0), inverted_projection.at(1, 0),
                                          inverted_projection.at(2, 0)};

            // Validate that the computed world position is reasonable
            constexpr float max_reasonable_component = 1e6f;
            if (!std::isfinite(world_pos.x) || !std::isfinite(world_pos.y)
                || !std::isfinite(world_pos.z) || std::abs(world_pos.x) > max_reasonable_component
                || std::abs(world_pos.y) > max_reasonable_component || std::abs(world_pos.z) > max_reasonable_component) {
                return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);
            }

            return world_pos;
        }

        template<ScreenStart screen_start = ScreenStart::TOP_LEFT_CORNER>
        [[nodiscard]]
        std::expected<Vector3<float>, Error> screen_to_world(const Vector3<float>& screen_pos) const noexcept
        {
            return view_port_to_screen(screen_to_ndc<screen_start>(screen_pos));
        }

        template<ScreenStart screen_start = ScreenStart::TOP_LEFT_CORNER>
        [[nodiscard]]
        std::expected<Vector3<float>, Error> screen_to_world(const Vector2<float>& screen_pos) const noexcept
        {
            const auto& [x, y] = screen_pos;
            return screen_to_world<screen_start>({x, y, 1.f});
        }

    protected:
        ViewPort m_view_port{};
        Angle<float, 0.f, 180.f, AngleFlags::Clamped> m_field_of_view;

        mutable std::optional<Mat4X4Type> m_view_projection_matrix;

        float m_far_plane_distance;
        float m_near_plane_distance;

        ViewAnglesType m_view_angles;
        Vector3<float> m_origin;

    private:
        template<class Type>
        [[nodiscard]] constexpr static bool is_ndc_out_of_bounds(const Type& ndc) noexcept
        {
            constexpr auto eps = std::numeric_limits<float>::epsilon();
            return std::ranges::any_of(ndc.raw_array(),
                                       [](const auto& val) { return val < -1.0f - eps || val > 1.0f + eps; });
        }

        // NDC REPRESENTATION:
        /*
                                ^
                                |        y
                            1   |
                                |
                                |
                    -1 ---------0--------- 1  --> x
                                |
                                |
                           -1   |
                                v
            */

        [[nodiscard]] Vector3<float>
        ndc_to_screen_position_from_top_left_corner(const Vector3<float>& ndc) const noexcept
        {
            /*
            +------------------------>
            | (0, 0)
            |
            |
            |
            |
            |
            |
            ⌄
            */
            return {(ndc.x + 1.f) / 2.f * m_view_port.m_width, (ndc.y / -2.f + 0.5f) * m_view_port.m_height, ndc.z};
        }

        [[nodiscard]] Vector3<float>
        ndc_to_screen_position_from_bottom_left_corner(const Vector3<float>& ndc) const noexcept
        {
            /*
             ^
             |
             |
             |
             |
             |
             |
             | (0, 0)
             +------------------------>
             */
            return {(ndc.x + 1.f) / 2.f * m_view_port.m_width, (ndc.y / 2.f + 0.5f) * m_view_port.m_height, ndc.z};
        }

        template<ScreenStart screen_start = ScreenStart::TOP_LEFT_CORNER>
        [[nodiscard]] Vector3<float> screen_to_ndc(const Vector3<float>& screen_pos) const noexcept
        {
            if constexpr (screen_start == ScreenStart::TOP_LEFT_CORNER)
                return {screen_pos.x / m_view_port.m_width * 2.f - 1.f, 1.f - screen_pos.y / m_view_port.m_height * 2.f,
                        screen_pos.z};
            else if constexpr (screen_start == ScreenStart::BOTTOM_LEFT_CORNER)
                return {screen_pos.x / m_view_port.m_width * 2.f - 1.f,
                        (screen_pos.y / m_view_port.m_height - 0.5f) * 2.f, screen_pos.z};
            else
                std::unreachable();
        }
    };
} // namespace omath::projection
