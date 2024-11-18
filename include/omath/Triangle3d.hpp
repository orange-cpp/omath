//
// Created by Orange on 11/13/2024.
//
#pragma once
#include "omath/Vector3.hpp"

namespace omath
{
    class Triangle3d final
    {
    public:
        Triangle3d(const Vector3& vertex1, const Vector3& vertex2, const Vector3& vertex3);
        Vector3 m_vertex1;
        Vector3 m_vertex2;
        Vector3 m_vertex3;

        [[nodiscard]]
        Vector3 CalculateNormal() const;

        [[nodiscard]]
        float SideALength() const;

        [[nodiscard]]
        float SideBLength() const;

        [[nodiscard]]
        Vector3 SideAVector() const;

        [[nodiscard]]
        Vector3 SideBVector() const;
    };
}