//
// Created by Vlad on 27.08.2024.
//

#pragma once

#include "omath/projection/error_codes.hpp"
#include <expected>
#include <omath/angle.hpp>
#include <omath/mat.hpp>
#include <omath/vector3.hpp>
#include <type_traits>

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

    template<class Mat4X4Type, class ViewAnglesType, class TraitClass>
    class Camera
    {
    public:
        virtual ~Camera() = default;
        Camera(const Vector3<float>& position, const ViewAnglesType& view_angles, const ViewPort& view_port,
               const FieldOfView& fov, const float near, const float far) noexcept
            : m_view_port(view_port), m_field_of_view(fov), m_far_plane_distance(far), m_near_plane_distance(near),
              m_view_angles(view_angles), m_origin(position)
        {
        }

    protected:
        void look_at(const Vector3<float>& target)
        {
            m_view_angles = TraitClass::calc_look_at_angle(m_origin, target);
        }

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

        [[nodiscard]] std::expected<Vector3<float>, Error>
        world_to_screen(const Vector3<float>& world_position) const noexcept
        {
            auto normalized_cords = world_to_view_port(world_position);

            if (!normalized_cords.has_value())
                return std::unexpected{normalized_cords.error()};

            return ndc_to_screen_position(*normalized_cords);
        }

        [[nodiscard]] std::expected<Vector3<float>, Error>
        world_to_view_port(const Vector3<float>& world_position) const noexcept
        {
            auto projected = get_view_projection_matrix()
                             * mat_column_from_vector<float, Mat4X4Type::get_store_ordering()>(world_position);

            if (projected.at(3, 0) == 0.0f)
                return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);

            projected /= projected.at(3, 0);

            if (is_ndc_out_of_bounds(projected))
                return std::unexpected(Error::WORLD_POSITION_IS_OUT_OF_SCREEN_BOUNDS);

            return Vector3<float>{projected.at(0, 0), projected.at(1, 0), projected.at(2, 0)};
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
            return std::ranges::any_of(ndc.raw_array(), [](const auto& val) { return val < -1 || val > 1; });
        }

        [[nodiscard]] Vector3<float> ndc_to_screen_position(const Vector3<float>& ndc) const noexcept
        {
            return {(ndc.x + 1.f) / 2.f * m_view_port.m_width, (1.f - ndc.y) / 2.f * m_view_port.m_height, ndc.z};
        }
    };
} // namespace omath::projection
