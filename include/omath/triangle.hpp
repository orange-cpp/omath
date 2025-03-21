//
// Created by Orange on 11/13/2024.
//
#pragma once
#include "omath/vector3.hpp"

namespace omath
{
        /*
        |\
        | \
      a |  \ hypot
        |   \
        -----
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

        Vector3<float> m_vertex1;
        Vector3<float> m_vertex2;
        Vector3<float> m_vertex3;

        [[nodiscard]]
        constexpr Vector3<float> CalculateNormal() const
        {
            const auto b = SideBVector();
            const auto a = SideAVector();
            return b.Cross(a).Normalized();
        }

        [[nodiscard]]
        float SideALength() const
        {
            return m_vertex1.DistTo(m_vertex2);
        }

        [[nodiscard]]
        float SideBLength() const
        {
            return m_vertex3.DistTo(m_vertex2);
        }

        [[nodiscard]]
        constexpr Vector3<float> SideAVector() const
        {
            return m_vertex1 - m_vertex2;
        }

        [[nodiscard]]
        constexpr float Hypot() const
        {
            return m_vertex1.DistTo(m_vertex3);
        }
        [[nodiscard]]
        constexpr bool IsRectangular() const
        {
            const auto sideA = SideALength();
            const auto sideB = SideBLength();
            const auto hypot = Hypot();

            return std::abs(sideA*sideA + sideB*sideB - hypot*hypot) <= 0.0001f;
        }
        [[nodiscard]]
        constexpr Vector3<float> SideBVector() const
        {
            return m_vertex3 - m_vertex2;
        }
        [[nodiscard]]
        constexpr Vector3<float> MidPoint() const
        {
            return (m_vertex1 + m_vertex2 + m_vertex3) / 3;
        }
    };
} // namespace omath
