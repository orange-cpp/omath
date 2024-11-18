#include "omath/Triangle3d.hpp"


namespace omath
{
    Triangle3d::Triangle3d(const Vector3 &vertex1, const Vector3 &vertex2, const Vector3 &vertex3)
        : m_vertex1(vertex1), m_vertex2(vertex2), m_vertex3(vertex3)
    {

    }

    Vector3 Triangle3d::CalculateNormal() const
    {
        return (m_vertex1 - m_vertex2).Cross(m_vertex3 - m_vertex1).Normalized();
    }

    float Triangle3d::SideALength() const
    {
        return m_vertex1.DistTo(m_vertex2);
    }

    float Triangle3d::SideBLength() const
    {
        return m_vertex3.DistTo(m_vertex2);
    }

    Vector3 Triangle3d::SideAVector() const
    {
        return m_vertex1 - m_vertex2;
    }

    Vector3 Triangle3d::SideBVector() const
    {
        return m_vertex3 - m_vertex2;
    }
}
