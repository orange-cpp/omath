//
// Created by Vlad on 11/9/2025.
//

#pragma once
#include "omath/linear_algebra/vector3.hpp"
#include <array>

namespace omath::collision
{
    template<class VectorType = Vector3<float>>
    class Simplex
    {
        std::array<VectorType, 4> m_points;
        std::size_t m_size;

    public:
        constexpr Simplex(): m_size(0)
        {
        }

        constexpr Simplex& operator=(const std::initializer_list<VectorType>& list)
        {
            m_size = 0;

            for (const VectorType& point : list)
                m_points[m_size++] = point;

            return *this;
        }

        constexpr void push_front(const VectorType& point)
        {
            m_points = {point, m_points[0], m_points[1], m_points[2]};
            m_size = std::min<std::size_t>(m_size + 1, 4);
        }

        constexpr const VectorType& operator[](const std::size_t i) const
        {
            return m_points[i];
        }
        [[nodiscard]]
        constexpr std::size_t size() const
        {
            return m_size;
        }

        [[nodiscard]]
        constexpr auto begin() const
        {
            return m_points.begin();
        }
        [[nodiscard]]
        constexpr auto end() const
        {
            return m_points.end() - (4 - m_size);
        }
        [[nodiscard]]
        constexpr bool handle(VectorType& direction)
        {
            switch (m_points.size())
            {
            case 2:
                return handle_line(direction);
            case 3:
                return handle_triangle(direction);
            case 4:
                return handle_tetrahedron(direction);
            default:
                std::unreachable();
            }
        }
    private:
        [[nodiscard]]
        constexpr bool handle_line(VectorType& direction)
        {
            const auto& a = m_points[0];
            const auto& b = m_points[1];

            const auto ab = b - a;
            // ReSharper disable once CppTooWideScopeInitStatement
            const auto ao = -a;

            if (ab.point_to_same_direction(ao))
                direction = ab.cross(ao).cross(ab);
            else
            {
                *this = {a};
                direction = ao;
            }

            return false;
        }
        [[nodiscard]]
        constexpr bool handle_triangle(VectorType& direction)
        {
            const auto& a = m_points[0];
            const auto& b = m_points[1];
            const auto& c = m_points[2];

            const auto ab = b - a;
            const auto ac = c - a;
            const auto ao = -a;

            const auto abc = ab.cross(ac);

            if (abc.cross(ac).point_to_same_direction(ao))
            {
                if (ac.point_to_same_direction(ao))
                {
                    *this = {a, c};
                    direction = ac.cross(ao).cross(ac);

                    return false;
                }
                *this = {a, b};
                return handle_line(direction);
            }

            if (ab.cross(abc).point_to_same_direction(ao))
            {
                *this = {a, b};
                return handle_line(direction);
            }
            if (abc.point_to_same_direction(ao))
            {
                direction = abc;
            }
            else
            {
                *this = {a, c, b};
                direction = -abc;
            }

            return false;
        }
        [[nodiscard]]
        constexpr bool handle_tetrahedron(VectorType& direction)
        {
            const auto& a = m_points[0];
            const auto& b = m_points[1];
            const auto& c = m_points[2];
            const auto& d = m_points[3];

            const auto ab = b - a;
            const auto ac = c - a;
            const auto ad = d - a;
            const auto ao = -a;

            const auto abc = ab.cross(ac);
            const auto acd = ac.cross(ad);
            const auto adb = ad.cross(ab);

            if (abc.point_to_same_direction(ao))
            {
                *this = {a, b, c};
                return handle_triangle(direction);
            }

            if (acd.point_to_same_direction(ao))
            {
                *this = {a, c, d};
                return handle_triangle(direction);
            }

            if (adb.point_to_same_direction(ao))
            {
                *this = {a, d, b};
                return handle_triangle(direction);
            }
            return true;
        }
    };

} // namespace omath::collision