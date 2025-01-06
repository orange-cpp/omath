//
// Created by Orange on 11/13/2024.
//
#pragma once
#include "omath/Vector3.hpp"

namespace omath
{
    template<class Vector>
    class Triangle final
    {
    public:
        constexpr Triangle(const Vector& vertex1, const Vector& vertex2, const Vector& vertex3)
            : m_vertex1(vertex1), m_vertex2(vertex2), m_vertex3(vertex3)
        {

        }

        Vector3 m_vertex1;
        Vector3 m_vertex2;
        Vector3 m_vertex3;

        [[nodiscard]]
        constexpr Vector3 CalculateNormal() const
        {
            return (m_vertex1 - m_vertex2).Cross(m_vertex3 - m_vertex1).Normalized();
        }

        [[nodiscard]]
        constexpr float SideALength() const
        {
            return m_vertex1.DistTo(m_vertex2);
        }

        [[nodiscard]]
        constexpr float SideBLength() const
        {
            return m_vertex3.DistTo(m_vertex2);
        }

        [[nodiscard]]
        constexpr Vector3 SideAVector() const
        {
            return m_vertex1 - m_vertex2;
        }

        [[nodiscard]]
        constexpr Vector3 SideBVector() const
        {
            return m_vertex3 - m_vertex2;
        }

        [[nodiscard]]
        constexpr Vector3 MidPoint() const
        {
            return (m_vertex1 + m_vertex2 + m_vertex3) / 3;
        }
    };
} // namespace omath
