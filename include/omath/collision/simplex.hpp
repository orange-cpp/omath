//
// Created by Vlad on 11/9/2025.
//

#pragma once
#include "omath/linear_algebra/vector3.hpp"
#include <array>

namespace omath::collision
{
    class Simplex
    {
        std::array<Vector3<float>, 4> m_points;
        int m_size;

    public:
        Simplex(): m_size(0)
        {
        }

        Simplex& operator=(const std::initializer_list<Vector3<float>> list)
        {
            m_size = 0;

            for (const Vector3<float>& point : list)
                m_points[m_size++] = point;

            return *this;
        }

        void push_front(const Vector3<float>& point)
        {
            m_points = {point, m_points[0], m_points[1], m_points[2]};
            m_size = std::min(m_size + 1, 4);
        }

        Vector3<float>& operator[](const std::size_t i)
        {
            return m_points[i];
        }
        [[nodiscard]] std::size_t size() const
        {
            return m_size;
        }

        [[nodiscard]] auto begin() const
        {
            return m_points.begin();
        }
        [[nodiscard]] auto end() const
        {
            return m_points.end() - (4 - m_size);
        }
    };

    bool handle_line(Simplex& points, Vector3<float>& direction)
    {
        const Vector3<float>& a = points[0];
        const Vector3<float>& b = points[1];

        const Vector3<float> ab = b - a;
        // ReSharper disable once CppTooWideScopeInitStatement
        const Vector3<float> ao = -a;

        if (ab.point_to_same_direction(ao))
            direction = ab.cross(ao).cross(ab);
        else
        {
            points = {a};
            direction = ao;
        }

        return false;
    }

    bool handle_triangle(Simplex& points, Vector3<float>& direction)
    {
        const Vector3<float>& a = points[0];
        const Vector3<float>& b = points[1];
        const Vector3<float>& c = points[2];

        const Vector3<float> ab = b - a;
        const Vector3<float> ac = c - a;
        const Vector3<float> ao = -a;

        const Vector3<float> abc = ab.cross(ac);

        if (abc.cross(ac).point_to_same_direction(ao))
        {
            if (ac.point_to_same_direction(ao))
            {
                points = {a, c};
                direction = ac.cross(ao).cross(ac);

                return false;
            }
            return handle_line(points = {a, b}, direction);
        }

        if (ab.cross(abc).point_to_same_direction(ao))
            return handle_line(points = {a, b}, direction);

        if (abc.point_to_same_direction(ao))
        {
            direction = abc;
        }
        else
        {
            points = {a, c, b};
            direction = -abc;
        }

        return false;
    }

    bool handle_tetrahedron(Simplex& simplex, Vector3<float>& direction)
    {
        const auto& a = simplex[0];
        const auto& b = simplex[1];
        const auto& c = simplex[2];
        const auto& d = simplex[3];

        const Vector3<float> ab = b - a;
        const Vector3<float> ac = c - a;
        const Vector3<float> ad = d - a;
        const Vector3<float> ao = -a;

        const Vector3<float> abc = ab.cross(ac);
        const Vector3<float> acd = ac.cross(ad);
        const Vector3<float> adb = ad.cross(ab);

        if (abc.point_to_same_direction(ao))
            return handle_triangle(simplex = {a, b, c}, direction);

        if (acd.point_to_same_direction(ao))
            return handle_triangle(simplex = {a, c, d}, direction);

        if (adb.point_to_same_direction(ao))
            return handle_triangle(simplex = {a, d, b}, direction);

        return true;
    }

    [[nodiscard]]
    bool handle_simplex(Simplex& points, Vector3<float>& direction)
    {
        switch (points.size())
        {
        case 2:
            return handle_line(points, direction);
        case 3:
            return handle_triangle(points, direction);
        case 4:
            return handle_tetrahedron(points, direction);
        default:
            return false;
        }
    }
} // namespace omath::collision