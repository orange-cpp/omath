//
// Created by Vlad on 27.08.2024.
//

#pragma once

#include "omath/3d_primitives/aabb.hpp"
#include "omath/3d_primitives/obb.hpp"
#include "omath/internal/optional_constexpr_math.hpp"
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

        [[nodiscard("You must use aspect ratio")]] constexpr float aspect_ratio() const
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
        bool inverted_right = false;
    };

    template<class T, class MatType, class ViewAnglesType, class NumericType>
    concept CameraEngineConcept =
            requires(const Vector3<NumericType>& cam_origin, const Vector3<NumericType>& look_at,
                     const ViewAnglesType& angles, const FieldOfView& fov, const ViewPort& viewport, NumericType z_near,
                     NumericType z_far, NDCDepthRange ndc_depth_range) {
                // Presence + return types
                { T::calc_look_at_angle(cam_origin, look_at) } -> std::same_as<ViewAnglesType>;
                { T::calc_view_matrix(angles, cam_origin) } -> std::same_as<MatType>;
                { T::calc_projection_matrix(fov, viewport, z_near, z_far, ndc_depth_range) } -> std::same_as<MatType>;
                requires std::is_floating_point_v<NumericType>;
                // Enforce noexcept as in the trait declaration
                requires noexcept(T::calc_look_at_angle(cam_origin, look_at));
                requires noexcept(T::calc_view_matrix(angles, cam_origin));
                requires noexcept(T::calc_projection_matrix(fov, viewport, z_near, z_far, ndc_depth_range));
            };

    template<class Mat4X4Type, class ViewAnglesType, class TraitClass,
             NDCDepthRange depth_range = NDCDepthRange::NEGATIVE_ONE_TO_ONE, CameraAxes axes = {},
             class NumericType = float>
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
        constexpr Camera(const Vector3<NumericType>& position, const ViewAnglesType& view_angles,
                         const ViewPort& view_port, const FieldOfView& fov, const NumericType near,
                         const NumericType far) noexcept
            : m_view_port(view_port), m_field_of_view(fov), m_far_plane_distance(far), m_near_plane_distance(near),
              m_view_angles(view_angles), m_origin(position)
        {
        }

        struct ProjectionParams final
        {
            FieldOfView fov;
            NumericType aspect_ratio{};
        };

        // Recovers vertical FOV and aspect ratio from a perspective projection matrix
        // built by any of the engine traits.  Both variants (ZERO_TO_ONE and
        // NEGATIVE_ONE_TO_ONE) share the same m[0,0]/m[1,1] layout, so this works
        // regardless of the NDC depth range.
        [[nodiscard("You must use extracted projection params")]]
        OMATH_CONSTEXPR static ProjectionParams extract_projection_params(const Mat4X4Type& proj_matrix) noexcept
        {
            // m[1,1] == 1 / tan(fov/2)  =>  fov = 2 * atan(1 / m[1,1])
            const auto f = proj_matrix.at(1, 1);
            // m[0,0] == m[1,1] / aspect_ratio  =>  aspect = m[1,1] / m[0,0]
            const auto fov_radians = NumericType{2} * internal::atan(NumericType{1} / f);
            return {FieldOfView::from_radians(static_cast<typename FieldOfView::ArithmeticType>(fov_radians)),
                    f / proj_matrix.at(0, 0)};
        }

        [[nodiscard("You must use calculated view angles")]]
        OMATH_CONSTEXPR static ViewAnglesType calc_view_angles_from_view_matrix(const Mat4X4Type& view_matrix) noexcept
        {
            Vector3<NumericType> forward_vector = {view_matrix[2, 0], view_matrix[2, 1], view_matrix[2, 2]};
            if constexpr (axes.inverted_forward)
                forward_vector = -forward_vector;
            return TraitClass::calc_look_at_angle({}, forward_vector);
        }

        [[nodiscard("You must use calculated origin")]]
        constexpr static Vector3<NumericType> calc_origin_from_view_matrix(const Mat4X4Type& view_matrix) noexcept
        {
            // The view matrix is R * T(-origin), so the last column stores t = -R * origin.
            // Recovering origin: origin = -R^T * t
            return {
                    -(view_matrix[0, 0] * view_matrix[0, 3] + view_matrix[1, 0] * view_matrix[1, 3]
                      + view_matrix[2, 0] * view_matrix[2, 3]),
                    -(view_matrix[0, 1] * view_matrix[0, 3] + view_matrix[1, 1] * view_matrix[1, 3]
                      + view_matrix[2, 1] * view_matrix[2, 3]),
                    -(view_matrix[0, 2] * view_matrix[0, 3] + view_matrix[1, 2] * view_matrix[1, 3]
                      + view_matrix[2, 2] * view_matrix[2, 3]),
            };
        }

        OMATH_CONSTEXPR void look_at(const Vector3<NumericType>& target)
        {
            m_view_angles = TraitClass::calc_look_at_angle(m_origin, target);
            m_view_projection_matrix = std::nullopt;
            m_view_matrix = std::nullopt;
        }
        [[nodiscard("You must use calculated look-at angles")]]
        OMATH_CONSTEXPR ViewAnglesType calc_look_at_angles(const Vector3<NumericType>& look_to) const
        {
            return TraitClass::calc_look_at_angle(m_origin, look_to);
        }

        [[nodiscard("You must use forward vector")]]
        OMATH_CONSTEXPR Vector3<NumericType> get_forward() const noexcept
        {
            if consteval
            {
                const auto view_matrix = calc_view_matrix();
                return {view_matrix[2, 0], view_matrix[2, 1], view_matrix[2, 2]};
            }

            const auto& view_matrix = get_view_matrix();
            return {view_matrix[2, 0], view_matrix[2, 1], view_matrix[2, 2]};
        }

        [[nodiscard("You must use right vector")]]
        OMATH_CONSTEXPR Vector3<NumericType> get_right() const noexcept
        {
            if consteval
            {
                const auto view_matrix = calc_view_matrix();
                return {view_matrix[0, 0], view_matrix[0, 1], view_matrix[0, 2]};
            }

            const auto& view_matrix = get_view_matrix();
            return {view_matrix[0, 0], view_matrix[0, 1], view_matrix[0, 2]};
        }

        [[nodiscard("You must use up vector")]]
        OMATH_CONSTEXPR Vector3<NumericType> get_up() const noexcept
        {
            if consteval
            {
                const auto view_matrix = calc_view_matrix();
                return {view_matrix[1, 0], view_matrix[1, 1], view_matrix[1, 2]};
            }

            const auto& view_matrix = get_view_matrix();
            return {view_matrix[1, 0], view_matrix[1, 1], view_matrix[1, 2]};
        }
        [[nodiscard("You must use absolute forward vector")]]
        OMATH_CONSTEXPR Vector3<NumericType> get_abs_forward() const noexcept
        {
            if constexpr (axes.inverted_forward)
                return -get_forward();
            return get_forward();
        }

        [[nodiscard("You must use absolute right vector")]]
        OMATH_CONSTEXPR Vector3<NumericType> get_abs_right() const noexcept
        {
            if constexpr (axes.inverted_right)
                return -get_right();
            return get_right();
        }

        [[nodiscard("You must use absolute up vector")]]
        OMATH_CONSTEXPR Vector3<NumericType> get_abs_up() const noexcept
        {
            return get_up();
        }

        [[nodiscard("You must use calculated view-projection matrix")]]
        OMATH_CONSTEXPR Mat4X4Type calc_view_projection_matrix() const noexcept
        {
            return calc_projection_matrix() * calc_view_matrix();
        }

        [[nodiscard("You must use calculated view matrix")]]
        OMATH_CONSTEXPR Mat4X4Type calc_view_matrix() const noexcept
        {
            return TraitClass::calc_view_matrix(m_view_angles, m_origin);
        }

        [[nodiscard("You must use calculated projection matrix")]]
        OMATH_CONSTEXPR Mat4X4Type calc_projection_matrix() const noexcept
        {
            return TraitClass::calc_projection_matrix(
                    m_field_of_view, m_view_port, m_near_plane_distance, m_far_plane_distance, depth_range);
        }

        [[nodiscard("You must use view-projection matrix")]]
        OMATH_CONSTEXPR const Mat4X4Type& get_view_projection_matrix() const noexcept
        {
            if (!m_view_projection_matrix.has_value())
                m_view_projection_matrix = get_projection_matrix() * get_view_matrix();

            return m_view_projection_matrix.value();
        }

        [[nodiscard("You must use view matrix")]] OMATH_CONSTEXPR const Mat4X4Type& get_view_matrix() const noexcept
        {
            if (!m_view_matrix.has_value())
                m_view_matrix = calc_view_matrix();

            return m_view_matrix.value();
        }
        [[nodiscard("You must use projection matrix")]] OMATH_CONSTEXPR const Mat4X4Type& get_projection_matrix() const noexcept
        {
            if (!m_projection_matrix.has_value())
                m_projection_matrix = calc_projection_matrix();

            return m_projection_matrix.value();
        }

        constexpr void set_field_of_view(const FieldOfView& fov) noexcept
        {
            m_field_of_view = fov;
            m_view_projection_matrix = std::nullopt;
            m_projection_matrix = std::nullopt;
        }

        constexpr void set_near_plane(const NumericType near_plane) noexcept
        {
            m_near_plane_distance = near_plane;
            m_view_projection_matrix = std::nullopt;
            m_projection_matrix = std::nullopt;
        }

        constexpr void set_far_plane(const NumericType far_plane) noexcept
        {
            m_far_plane_distance = far_plane;
            m_view_projection_matrix = std::nullopt;
            m_projection_matrix = std::nullopt;
        }

        constexpr void set_view_angles(const ViewAnglesType& view_angles) noexcept
        {
            m_view_angles = view_angles;
            m_view_projection_matrix = std::nullopt;
            m_view_matrix = std::nullopt;
        }

        constexpr void set_origin(const Vector3<NumericType>& origin) noexcept
        {
            m_origin = origin;
            m_view_projection_matrix = std::nullopt;
            m_view_matrix = std::nullopt;
        }
        constexpr void set_view_port(const ViewPort& view_port) noexcept
        {
            m_view_port = view_port;
            m_view_projection_matrix = std::nullopt;
            m_projection_matrix = std::nullopt;
        }

        [[nodiscard("You must use field of view")]] constexpr const FieldOfView& get_field_of_view() const noexcept
        {
            return m_field_of_view;
        }

        [[nodiscard("You must use near plane")]] constexpr const NumericType& get_near_plane() const noexcept
        {
            return m_near_plane_distance;
        }

        [[nodiscard("You must use far plane")]] constexpr const NumericType& get_far_plane() const noexcept
        {
            return m_far_plane_distance;
        }

        [[nodiscard("You must use view angles")]] constexpr const ViewAnglesType& get_view_angles() const noexcept
        {
            return m_view_angles;
        }

        [[nodiscard("You must use origin")]] constexpr const Vector3<NumericType>& get_origin() const noexcept
        {
            return m_origin;
        }

        template<ScreenStart screen_start = ScreenStart::TOP_LEFT_CORNER>
        [[nodiscard("You must use screen position")]] OMATH_CONSTEXPR std::expected<Vector3<NumericType>, Error>
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
        [[nodiscard("You must use unclipped screen position")]] OMATH_CONSTEXPR std::expected<Vector3<NumericType>, Error>
        world_to_screen_unclipped(const Vector3<NumericType>& world_position) const noexcept
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

        [[nodiscard("You must use frustum culling result")]] OMATH_CONSTEXPR bool
        is_culled_by_frustum(const Triangle<Vector3<NumericType>>& triangle) const noexcept
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
            if (c0[3] <= NumericType{0} && c1[3] <= NumericType{0} && c2[3] <= NumericType{0})
                return true;

            // Helper: all three vertices outside the same clip plane
            auto all_outside_plane = [](const int axis, const std::array<NumericType, 4>& a,
                                        const std::array<NumericType, 4>& b, const std::array<NumericType, 4>& c,
                                        const bool positive_side)
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

        [[nodiscard("You must use AABB frustum culling result")]] OMATH_CONSTEXPR bool
        is_aabb_culled_by_frustum(const primitives::Aabb<NumericType>& aabb) const noexcept
        {
            // For each plane, find the AABB corner most in the direction of the plane normal
            // (the "positive vertex"). If it's outside, the entire AABB is outside.
            for (const auto& [a, b, c, d] : extract_frustum_planes())
            {
                const auto px = a >= NumericType{0} ? aabb.max.x : aabb.min.x;
                const auto py = b >= NumericType{0} ? aabb.max.y : aabb.min.y;
                const auto pz = c >= NumericType{0} ? aabb.max.z : aabb.min.z;

                if (a * px + b * py + c * pz + d < NumericType{0})
                    return true;
            }

            return false;
        }

        [[nodiscard("You must use OBB frustum culling result")]] OMATH_CONSTEXPR bool
        is_obb_culled_by_frustum(const primitives::Obb<NumericType>& obb) const noexcept
        {
            // For each plane, project the OBB extents onto the plane normal to get the
            // effective radius, then test the center's signed distance against it.
            for (const auto& [a, b, c, d] : extract_frustum_planes())
            {
                const Vector3<NumericType> normal{a, b, c};

                const auto center_distance = normal.dot(obb.center) + d;
                const auto radius = obb.half_extents.x * internal::abs(normal.dot(obb.axis_x))
                                    + obb.half_extents.y * internal::abs(normal.dot(obb.axis_y))
                                    + obb.half_extents.z * internal::abs(normal.dot(obb.axis_z));

                if (center_distance + radius < NumericType{0})
                    return true;
            }

            return false;
        }

        [[nodiscard("You must use view port position")]] OMATH_CONSTEXPR std::expected<Vector3<NumericType>, Error>
        world_to_view_port(const Vector3<NumericType>& world_position,
                           const ViewPortClipping& clipping = ViewPortClipping::AUTO) const noexcept
        {
            auto project_to_view_port = [this, &clipping](auto projected) -> std::expected<Vector3<NumericType>, Error>
            {
                const auto& w = projected.at(3, 0);
                constexpr auto eps = std::numeric_limits<NumericType>::epsilon();
                if (w <= eps)
                    return std::unexpected(Error::PERSPECTIVE_DIVIDER_LESS_EQ_ZERO);

                projected /= w;

                // ReSharper disable once CppTooWideScope
                const auto clipped_automatically =
                        clipping == ViewPortClipping::AUTO && is_ndc_out_of_bounds(projected);
                if (clipped_automatically)
                    return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);

                // ReSharper disable once CppTooWideScope
                constexpr auto z_min = depth_range == NDCDepthRange::ZERO_TO_ONE ? NumericType{0} : -NumericType{1};
                const auto clipped_manually =
                        clipping == ViewPortClipping::MANUAL
                        && (projected.at(2, 0) < z_min - eps || projected.at(2, 0) > NumericType{1} + eps);
                if (clipped_manually)
                    return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);

                return Vector3<NumericType>{projected.at(0, 0), projected.at(1, 0), projected.at(2, 0)};
            };

            if consteval
            {
                auto projected =
                        calc_view_projection_matrix()
                        * mat_column_from_vector<NumericType, Mat4X4Type::get_store_ordering()>(world_position);
                return project_to_view_port(projected);
            }

            auto projected = get_view_projection_matrix()
                             * mat_column_from_vector<NumericType, Mat4X4Type::get_store_ordering()>(world_position);
            return project_to_view_port(projected);
        }
        [[nodiscard("You must use world position")]]
        OMATH_CONSTEXPR std::expected<Vector3<NumericType>, Error>
        view_port_to_world(const Vector3<NumericType>& ndc) const noexcept
        {
            auto view_port_to_world = [&ndc](const Mat4X4Type& view_projection) -> std::expected<Vector3<NumericType>, Error>
            {
                const auto inv_view_proj = view_projection.inverted();

                if (!inv_view_proj)
                    return std::unexpected(Error::INV_VIEW_PROJ_MAT_DET_EQ_ZERO);

                auto inverted_projection = inv_view_proj.value()
                                           * mat_column_from_vector<NumericType, Mat4X4Type::get_store_ordering()>(ndc);

                const auto& w = inverted_projection.at(3, 0);

                if (internal::abs(w) < std::numeric_limits<NumericType>::epsilon())
                    return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);

                inverted_projection /= w;

                return Vector3<NumericType>{inverted_projection.at(0, 0), inverted_projection.at(1, 0),
                                            inverted_projection.at(2, 0)};
            };

            if consteval
            {
                return view_port_to_world(calc_view_projection_matrix());
            }

            return view_port_to_world(get_view_projection_matrix());
        }

        template<ScreenStart screen_start = ScreenStart::TOP_LEFT_CORNER>
        [[nodiscard("You must use world position")]]
        OMATH_CONSTEXPR std::expected<Vector3<NumericType>, Error>
        screen_to_world(const Vector3<NumericType>& screen_pos) const noexcept
        {
            return view_port_to_world(screen_to_ndc<screen_start>(screen_pos));
        }

        template<ScreenStart screen_start = ScreenStart::TOP_LEFT_CORNER>
        [[nodiscard("You must use world position")]]
        OMATH_CONSTEXPR std::expected<Vector3<NumericType>, Error>
        screen_to_world(const Vector2<NumericType>& screen_pos) const noexcept
        {
            const auto& [x, y] = screen_pos;
            return screen_to_world<screen_start>({x, y, 1});
        }

    protected:
        ViewPort m_view_port{};
        FieldOfView m_field_of_view;

        mutable std::optional<Mat4X4Type> m_view_projection_matrix;
        mutable std::optional<Mat4X4Type> m_projection_matrix;
        mutable std::optional<Mat4X4Type> m_view_matrix;
        NumericType m_far_plane_distance;
        NumericType m_near_plane_distance;

        ViewAnglesType m_view_angles;
        Vector3<NumericType> m_origin;

    private:
        struct FrustumPlane final
        {
            NumericType a, b, c, d;
        };

        // Gribb-Hartmann: extract 6 frustum planes from the view-projection matrix.
        // Each plane is (a, b, c, d) such that ax + by + cz + d >= 0 means inside.
        // For a 4x4 matrix with rows r0..r3:
        //   Left   = r3 + r0
        //   Right  = r3 - r0
        //   Bottom = r3 + r1
        //   Top    = r3 - r1
        //   Near   = r3 + r2  ([-1,1]) or r2 ([0,1])
        //   Far    = r3 - r2
        [[nodiscard("You must use frustum planes")]] OMATH_CONSTEXPR std::array<FrustumPlane, 6>
        extract_frustum_planes() const noexcept
        {
            const auto& m = get_view_projection_matrix();

            const auto extract_plane = [&m](const int sign, const int row) -> FrustumPlane
            {
                return {
                        m.at(3, 0) + static_cast<NumericType>(sign) * m.at(row, 0),
                        m.at(3, 1) + static_cast<NumericType>(sign) * m.at(row, 1),
                        m.at(3, 2) + static_cast<NumericType>(sign) * m.at(row, 2),
                        m.at(3, 3) + static_cast<NumericType>(sign) * m.at(row, 3),
                };
            };

            std::array<FrustumPlane, 6> planes = {
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

            return planes;
        }

        template<class Type>
        [[nodiscard("You must use NDC bounds check result")]] constexpr static bool
        is_ndc_out_of_bounds(const Type& ndc) noexcept
        {
            constexpr auto eps = std::numeric_limits<NumericType>::epsilon();

            const auto& data = ndc.raw_array();
            // x and y are always in [-1, 1]
            if (data[0] < -NumericType{1} - eps || data[0] > NumericType{1} + eps)
                return true;
            if (data[1] < -NumericType{1} - eps || data[1] > NumericType{1} + eps)
                return true;
            return is_ndc_z_value_out_of_bounds(data[2]);
        }
        template<class ZType>
        [[nodiscard("You must use NDC z bounds check result")]]
        constexpr static bool is_ndc_z_value_out_of_bounds(const ZType& z_ndc) noexcept
        {
            constexpr auto eps = std::numeric_limits<NumericType>::epsilon();
            if constexpr (depth_range == NDCDepthRange::NEGATIVE_ONE_TO_ONE)
                return z_ndc < -NumericType{1} - eps || z_ndc > NumericType{1} + eps;
            if constexpr (depth_range == NDCDepthRange::ZERO_TO_ONE)
                return z_ndc < NumericType{0} - eps || z_ndc > NumericType{1} + eps;

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

        [[nodiscard("You must use screen position")]] constexpr Vector3<NumericType>
        ndc_to_screen_position_from_top_left_corner(const Vector3<NumericType>& ndc) const noexcept
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
            return {(ndc.x + NumericType{1}) / NumericType{2} * m_view_port.m_width,
                    (ndc.y / -NumericType{2} + NumericType{0.5}) * m_view_port.m_height, ndc.z};
        }

        [[nodiscard("You must use screen position")]] constexpr Vector3<NumericType>
        ndc_to_screen_position_from_bottom_left_corner(const Vector3<NumericType>& ndc) const noexcept
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
            return {(ndc.x + NumericType{1}) / NumericType{2} * m_view_port.m_width,
                    (ndc.y / NumericType{2} + NumericType{0.5}) * m_view_port.m_height, ndc.z};
        }

        template<ScreenStart screen_start = ScreenStart::TOP_LEFT_CORNER>
        [[nodiscard("You must use NDC position")]] constexpr Vector3<NumericType>
        screen_to_ndc(const Vector3<NumericType>& screen_pos) const noexcept
        {
            if constexpr (screen_start == ScreenStart::TOP_LEFT_CORNER)
                return {screen_pos.x / m_view_port.m_width * NumericType{2} - NumericType{1},
                        NumericType{1} - screen_pos.y / m_view_port.m_height * NumericType{2}, screen_pos.z};
            else if constexpr (screen_start == ScreenStart::BOTTOM_LEFT_CORNER)
                return {screen_pos.x / m_view_port.m_width * NumericType{2} - NumericType{1},
                        (screen_pos.y / m_view_port.m_height - NumericType{0.5}) * NumericType{2}, screen_pos.z};
            else
                std::unreachable();
        }
    };
} // namespace omath::projection
