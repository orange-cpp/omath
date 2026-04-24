//
// Created by Vlad on 27.08.2024.
//

#pragma once

#include "omath/3d_primitives/aabb.hpp"
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
    enum class ViewPortClipping
    {
        AUTO,
        MANUAL,
    };
    struct CameraAxes
    {
        bool inverted_forward = false;
        bool inverted_right   = false;
    };

    template<class T, class MatType, class ViewAnglesType, class NumericType>
    concept CameraEngineConcept =
            requires(const Vector3<float>& cam_origin, const Vector3<float>& look_at, const ViewAnglesType& angles,
                     const FieldOfView& fov, const ViewPort& viewport, float znear, float zfar,
                     NDCDepthRange ndc_depth_range) {
                // Presence + return types
                { T::calc_look_at_angle(cam_origin, look_at) } -> std::same_as<ViewAnglesType>;
                { T::calc_view_matrix(angles, cam_origin) } -> std::same_as<MatType>;
                { T::calc_projection_matrix(fov, viewport, znear, zfar, ndc_depth_range) } -> std::same_as<MatType>;
                requires std::is_floating_point_v<NumericType>;
                // Enforce noexcept as in the trait declaration
                requires noexcept(T::calc_look_at_angle(cam_origin, look_at));
                requires noexcept(T::calc_view_matrix(angles, cam_origin));
                requires noexcept(T::calc_projection_matrix(fov, viewport, znear, zfar, ndc_depth_range));
            };

    template<class Mat4X4Type, class ViewAnglesType, class TraitClass,
             NDCDepthRange depth_range = NDCDepthRange::NEGATIVE_ONE_TO_ONE,
             CameraAxes axes = {}, class NumericType = float>
    requires CameraEngineConcept<TraitClass, Mat4X4Type, ViewAnglesType, NumericType>
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
               const FieldOfView& fov, const NumericType near, const NumericType far) noexcept
            : m_view_port(view_port), m_field_of_view(fov), m_far_plane_distance(far), m_near_plane_distance(near),
              m_view_angles(view_angles), m_origin(position)
        {
        }

        struct ProjectionParams
        {
            FieldOfView fov;
            float aspect_ratio;
        };

        // Recovers vertical FOV and aspect ratio from a perspective projection matrix
        // built by any of the engine traits.  Both variants (ZERO_TO_ONE and
        // NEGATIVE_ONE_TO_ONE) share the same m[0,0]/m[1,1] layout, so this works
        // regardless of the NDC depth range.
        [[nodiscard]]
        static ProjectionParams extract_projection_params(const Mat4X4Type& proj_matrix) noexcept
        {
            // m[1,1] == 1 / tan(fov/2)  =>  fov = 2 * atan(1 / m[1,1])
            const auto f = proj_matrix.at(1, 1);
            // m[0,0] == m[1,1] / aspect_ratio  =>  aspect = m[1,1] / m[0,0]
            return {FieldOfView::from_radians(2.f * std::atan(1.f / f)), f / proj_matrix.at(0, 0)};
        }

        [[nodiscard]]
        static ViewAnglesType calc_view_angles_from_view_matrix(const Mat4X4Type& view_matrix) noexcept
        {
            Vector3<NumericType> forward_vector = {view_matrix[2, 0], view_matrix[2, 1], view_matrix[2, 2]};
            if constexpr (axes.inverted_forward)
                forward_vector = -forward_vector;
            return TraitClass::calc_look_at_angle({}, forward_vector);
        }

        [[nodiscard]]
        static Vector3<NumericType> calc_origin_from_view_matrix(const Mat4X4Type& view_matrix) noexcept
        {
            // The view matrix is R * T(-origin), so the last column stores t = -R * origin.
            // Recovering origin: origin = -R^T * t
            return {
                -(view_matrix[0, 0] * view_matrix[0, 3] + view_matrix[1, 0] * view_matrix[1, 3] + view_matrix[2, 0] * view_matrix[2, 3]),
                -(view_matrix[0, 1] * view_matrix[0, 3] + view_matrix[1, 1] * view_matrix[1, 3] + view_matrix[2, 1] * view_matrix[2, 3]),
                -(view_matrix[0, 2] * view_matrix[0, 3] + view_matrix[1, 2] * view_matrix[1, 3] + view_matrix[2, 2] * view_matrix[2, 3]),
            };
        }

        void look_at(const Vector3<NumericType>& target)
        {
            m_view_angles = TraitClass::calc_look_at_angle(m_origin, target);
            m_view_projection_matrix = std::nullopt;
            m_view_matrix = std::nullopt;
        }
        [[nodiscard]]
        ViewAnglesType calc_look_at_angles(const Vector3<NumericType>& look_to) const
        {
            return TraitClass::calc_look_at_angle(m_origin, look_to);
        }

        [[nodiscard]]
        Vector3<NumericType> get_forward() const noexcept
        {
            const auto& view_matrix = get_view_matrix();
            return {view_matrix[2, 0], view_matrix[2, 1], view_matrix[2, 2]};
        }

        [[nodiscard]]
        Vector3<NumericType> get_right() const noexcept
        {
            const auto& view_matrix = get_view_matrix();
            return {view_matrix[0, 0], view_matrix[0, 1], view_matrix[0, 2]};
        }

        [[nodiscard]]
        Vector3<NumericType> get_up() const noexcept
        {
            const auto& view_matrix = get_view_matrix();
            return {view_matrix[1, 0], view_matrix[1, 1], view_matrix[1, 2]};
        }
        [[nodiscard]]
        Vector3<NumericType> get_abs_forward() const noexcept
        {
            if constexpr (axes.inverted_forward)
                return -get_forward();
            return get_forward();
        }

        [[nodiscard]]
        Vector3<NumericType> get_abs_right() const noexcept
        {
            if constexpr (axes.inverted_right)
                return -get_right();
            return get_right();
        }

        [[nodiscard]]
        Vector3<NumericType> get_abs_up() const noexcept
        {
            return get_up();
        }

        [[nodiscard]] const Mat4X4Type& get_view_projection_matrix() const noexcept
        {
            if (!m_view_projection_matrix.has_value())
                m_view_projection_matrix = get_projection_matrix() * get_view_matrix();

            return m_view_projection_matrix.value();
        }

        [[nodiscard]] const Mat4X4Type& get_view_matrix() const noexcept
        {
            if (!m_view_matrix.has_value())
                m_view_matrix = TraitClass::calc_view_matrix(m_view_angles, m_origin);

            return m_view_matrix.value();
        }
        [[nodiscard]] const Mat4X4Type& get_projection_matrix() const noexcept
        {
            if (!m_projection_matrix.has_value())
                m_projection_matrix = TraitClass::calc_projection_matrix(m_field_of_view, m_view_port,
                                                                         m_near_plane_distance, m_far_plane_distance,
                                                                         depth_range);

            return m_projection_matrix.value();
        }

        void set_field_of_view(const FieldOfView& fov) noexcept
        {
            m_field_of_view = fov;
            m_view_projection_matrix = std::nullopt;
            m_projection_matrix = std::nullopt;
        }

        void set_near_plane(const NumericType near_plane) noexcept
        {
            m_near_plane_distance = near_plane;
            m_view_projection_matrix = std::nullopt;
            m_projection_matrix = std::nullopt;
        }

        void set_far_plane(const NumericType far_plane) noexcept
        {
            m_far_plane_distance = far_plane;
            m_view_projection_matrix = std::nullopt;
            m_projection_matrix = std::nullopt;
        }

        void set_view_angles(const ViewAnglesType& view_angles) noexcept
        {
            m_view_angles = view_angles;
            m_view_projection_matrix = std::nullopt;
            m_view_matrix = std::nullopt;
        }

        void set_origin(const Vector3<NumericType>& origin) noexcept
        {
            m_origin = origin;
            m_view_projection_matrix = std::nullopt;
            m_view_matrix = std::nullopt;
        }
        void set_view_port(const ViewPort& view_port) noexcept
        {
            m_view_port = view_port;
            m_view_projection_matrix = std::nullopt;
            m_projection_matrix = std::nullopt;
        }

        [[nodiscard]] const FieldOfView& get_field_of_view() const noexcept
        {
            return m_field_of_view;
        }

        [[nodiscard]] const NumericType& get_near_plane() const noexcept
        {
            return m_near_plane_distance;
        }

        [[nodiscard]] const NumericType& get_far_plane() const noexcept
        {
            return m_far_plane_distance;
        }

        [[nodiscard]] const ViewAnglesType& get_view_angles() const noexcept
        {
            return m_view_angles;
        }

        [[nodiscard]] const Vector3<NumericType>& get_origin() const noexcept
        {
            return m_origin;
        }

        template<ScreenStart screen_start = ScreenStart::TOP_LEFT_CORNER>
        [[nodiscard]] std::expected<Vector3<NumericType>, Error>
        world_to_screen(const Vector3<NumericType>& world_position) const noexcept
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
        template<ScreenStart screen_start = ScreenStart::TOP_LEFT_CORNER>
        [[nodiscard]] std::expected<Vector3<NumericType>, Error>
        world_to_screen_unclipped(const Vector3<float>& world_position) const noexcept
        {
            const auto normalized_cords = world_to_view_port(world_position, ViewPortClipping::MANUAL);

            if (!normalized_cords.has_value())
                return std::unexpected{normalized_cords.error()};

            if constexpr (screen_start == ScreenStart::TOP_LEFT_CORNER)
                return ndc_to_screen_position_from_top_left_corner(*normalized_cords);
            else if constexpr (screen_start == ScreenStart::BOTTOM_LEFT_CORNER)
                return ndc_to_screen_position_from_bottom_left_corner(*normalized_cords);
            else
                std::unreachable();
        }

        [[nodiscard]] bool is_culled_by_frustum(const Triangle<Vector3<NumericType>>& triangle) const noexcept
        {
            // Transform to clip space (before perspective divide)
            auto to_clip = [this](const Vector3<NumericType>& point)
            {
                auto clip = get_view_projection_matrix()
                            * mat_column_from_vector<NumericType, Mat4X4Type::get_store_ordering()>(point);
                return std::array<NumericType, 4>{
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
            auto all_outside_plane = [](const int axis, const std::array<NumericType, 4>& a, const std::array<NumericType, 4>& b,
                                        const std::array<NumericType, 4>& c, const bool positive_side)
            {
                if (positive_side)
                    return a[axis] > a[3] && b[axis] > b[3] && c[axis] > c[3];
                return a[axis] < -a[3] && b[axis] < -b[3] && c[axis] < -c[3];
            };

            // Clip volume in clip space:
            // -w <= x <= w
            // -w <= y <= w
            // z_min <= z <= w  (z_min = -w for [-1,1], 0 for [0,1])

            // x and y planes
            for (int i = 0; i < 2; i++)
            {
                if (all_outside_plane(i, c0, c1, c2, false))
                    return true;
                if (all_outside_plane(i, c0, c1, c2, true))
                    return true;
            }

            // z far plane: z > w
            if (all_outside_plane(2, c0, c1, c2, true))
                return true;

            // z near plane
            if constexpr (depth_range == NDCDepthRange::ZERO_TO_ONE)
            {
                // 0 <= z, so reject if z < 0 for all vertices
                if (c0[2] < 0.f && c1[2] < 0.f && c2[2] < 0.f)
                    return true;
            }
            else
            {
                // -w <= z
                if (all_outside_plane(2, c0, c1, c2, false))
                    return true;
            }
            return false;
        }

        [[nodiscard]] bool is_aabb_culled_by_frustum(const primitives::Aabb<NumericType>& aabb) const noexcept
        {
            const auto& m = get_view_projection_matrix();

            // Gribb-Hartmann: extract 6 frustum planes from the view-projection matrix.
            // Each plane is (a, b, c, d) such that ax + by + cz + d >= 0 means inside.
            // For a 4x4 matrix with rows r0..r3:
            //   Left   = r3 + r0
            //   Right  = r3 - r0
            //   Bottom = r3 + r1
            //   Top    = r3 - r1
            //   Near   = r3 + r2  ([-1,1]) or r2 ([0,1])
            //   Far    = r3 - r2
            struct Plane final
            {
                float a, b, c, d;
            };

            const auto extract_plane = [&m](const int sign, const int row) -> Plane
            {
                return {
                        m.at(3, 0) + static_cast<float>(sign) * m.at(row, 0),
                        m.at(3, 1) + static_cast<float>(sign) * m.at(row, 1),
                        m.at(3, 2) + static_cast<float>(sign) * m.at(row, 2),
                        m.at(3, 3) + static_cast<float>(sign) * m.at(row, 3),
                };
            };

            std::array<Plane, 6> planes = {
                    extract_plane(1, 0), // left
                    extract_plane(-1, 0), // right
                    extract_plane(1, 1), // bottom
                    extract_plane(-1, 1), // top
                    extract_plane(-1, 2), // far
            };

            // Near plane depends on NDC depth range
            if constexpr (depth_range == NDCDepthRange::ZERO_TO_ONE)
                planes[5] = {m.at(2, 0), m.at(2, 1), m.at(2, 2), m.at(2, 3)};
            else
                planes[5] = extract_plane(1, 2);

            // For each plane, find the AABB corner most in the direction of the plane normal
            // (the "positive vertex"). If it's outside, the entire AABB is outside.
            for (const auto& [a, b, c, d] : planes)
            {
                const auto px = a >= 0.f ? aabb.max.x : aabb.min.x;
                const auto py = b >= 0.f ? aabb.max.y : aabb.min.y;
                const auto pz = c >= 0.f ? aabb.max.z : aabb.min.z;

                if (a * px + b * py + c * pz + d < 0.f)
                    return true;
            }

            return false;
        }

        [[nodiscard]] std::expected<Vector3<NumericType>, Error>
        world_to_view_port(const Vector3<NumericType>& world_position,
                           const ViewPortClipping& clipping = ViewPortClipping::AUTO) const noexcept
        {
            auto projected = get_view_projection_matrix()
                             * mat_column_from_vector<NumericType, Mat4X4Type::get_store_ordering()>(world_position);

            const auto& w = projected.at(3, 0);
            constexpr auto eps = std::numeric_limits<NumericType>::epsilon();
            if (w <= eps)
                return std::unexpected(Error::PERSPECTIVE_DIVIDER_LESS_EQ_ZERO);

            projected /= w;

            // ReSharper disable once CppTooWideScope
            const auto clipped_automatically = clipping == ViewPortClipping::AUTO && is_ndc_out_of_bounds(projected);
            if (clipped_automatically)
                return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);

            // ReSharper disable once CppTooWideScope
            constexpr auto z_min = depth_range == NDCDepthRange::ZERO_TO_ONE ? 0.0f : -1.0f;
            const auto clipped_manually = clipping == ViewPortClipping::MANUAL && (projected.at(2, 0) < z_min - eps
                                          || projected.at(2, 0) > 1.0f + eps);
            if (clipped_manually)
                return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);

            return Vector3<float>{projected.at(0, 0), projected.at(1, 0), projected.at(2, 0)};
        }
        [[nodiscard]]
        std::expected<Vector3<NumericType>, Error> view_port_to_world(const Vector3<NumericType>& ndc) const noexcept
        {
            const auto inv_view_proj = get_view_projection_matrix().inverted();

            if (!inv_view_proj)
                return std::unexpected(Error::INV_VIEW_PROJ_MAT_DET_EQ_ZERO);

            auto inverted_projection =
                    inv_view_proj.value() * mat_column_from_vector<NumericType, Mat4X4Type::get_store_ordering()>(ndc);

            const auto& w = inverted_projection.at(3, 0);

            if (std::abs(w) < std::numeric_limits<NumericType>::epsilon())
                return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);

            inverted_projection /= w;

            return Vector3<NumericType>{inverted_projection.at(0, 0), inverted_projection.at(1, 0),
                                  inverted_projection.at(2, 0)};
        }

        template<ScreenStart screen_start = ScreenStart::TOP_LEFT_CORNER>
        [[nodiscard]]
        std::expected<Vector3<NumericType>, Error> screen_to_world(const Vector3<NumericType>& screen_pos) const noexcept
        {
            return view_port_to_world(screen_to_ndc<screen_start>(screen_pos));
        }

        template<ScreenStart screen_start = ScreenStart::TOP_LEFT_CORNER>
        [[nodiscard]]
        std::expected<Vector3<NumericType>, Error> screen_to_world(const Vector2<NumericType>& screen_pos) const noexcept
        {
            const auto& [x, y] = screen_pos;
            return screen_to_world<screen_start>({x, y, 1.f});
        }

    protected:
        ViewPort m_view_port{};
        Angle<NumericType, 0.f, 180.f, AngleFlags::Clamped> m_field_of_view;

        mutable std::optional<Mat4X4Type> m_view_projection_matrix;
        mutable std::optional<Mat4X4Type> m_projection_matrix;
        mutable std::optional<Mat4X4Type> m_view_matrix;
        float m_far_plane_distance;
        float m_near_plane_distance;

        ViewAnglesType m_view_angles;
        Vector3<NumericType> m_origin;

    private:
        template<class Type>
        [[nodiscard]] constexpr static bool is_ndc_out_of_bounds(const Type& ndc) noexcept
        {
            constexpr auto eps = std::numeric_limits<NumericType>::epsilon();

            const auto& data = ndc.raw_array();
            // x and y are always in [-1, 1]
            if (data[0] < -1.0f - eps || data[0] > 1.0f + eps)
                return true;
            if (data[1] < -1.0f - eps || data[1] > 1.0f + eps)
                return true;
            return is_ndc_z_value_out_of_bounds(data[2]);
        }
        template<class ZType>
         [[nodiscard]]
        constexpr static bool is_ndc_z_value_out_of_bounds(const ZType& z_ndc) noexcept
        {
            constexpr auto eps = std::numeric_limits<float>::epsilon();
            if constexpr (depth_range == NDCDepthRange::NEGATIVE_ONE_TO_ONE)
                return z_ndc < -1.0f - eps || z_ndc > 1.0f + eps;
            if constexpr (depth_range == NDCDepthRange::ZERO_TO_ONE)
                return z_ndc < 0.0f - eps || z_ndc > 1.0f + eps;

            std::unreachable();
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
