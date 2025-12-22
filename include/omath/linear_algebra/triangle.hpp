//
// Created by Orange on 11/13/2024.
//
#pragma once
#include "vector3.hpp"

namespace omath
{
    /*
    v1
    |\
    | \
  a |  \ hypot
    |   \
 v2 ----- v3
      b
    */

    template<class Vector>
    class Triangle final
    {
    public:
        constexpr Triangle() = default;
        constexpr Triangle(const Vector& vertex1, const Vector& vertex2, const Vector& vertex3)
            : m_vertex1(vertex1), m_vertex2(vertex2), m_vertex3(vertex3)
        {
        }

        Vector m_vertex1;
        Vector m_vertex2;
        Vector m_vertex3;

        [[nodiscard]]
        constexpr Vector calculate_normal() const
        {
            const auto b = side_b_vector();
            const auto a = side_a_vector();

            return b.cross(a).normalized();
        }

        [[nodiscard]]
        Vector::ContainedType side_a_length() const
        {
            return m_vertex1.distance_to(m_vertex2);
        }

        [[nodiscard]]
        Vector::ContainedType side_b_length() const
        {
            return m_vertex3.distance_to(m_vertex2);
        }

        [[nodiscard]]
        constexpr Vector side_a_vector() const
        {
            return m_vertex1 - m_vertex2;
        }

        [[nodiscard]]
        constexpr Vector::ContainedType hypot() const
        {
            return m_vertex1.distance_to(m_vertex3);
        }
        [[nodiscard]]
        constexpr bool is_rectangular() const
        {
            const auto side_a = side_a_length();
            const auto side_b = side_b_length();
            const auto hypot_value = hypot();

            return std::abs(side_a * side_a + side_b * side_b - hypot_value * hypot_value) <= 0.0001f;
        }
        [[nodiscard]]
        constexpr Vector side_b_vector() const
        {
            return m_vertex3 - m_vertex2;
        }
        [[nodiscard]]
        constexpr Vector mid_point() const
        {
            return (m_vertex1 + m_vertex2 + m_vertex3) / 3;
        }
    };


} // namespace omath
