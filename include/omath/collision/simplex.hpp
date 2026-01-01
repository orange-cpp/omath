#pragma once
#include "omath/linear_algebra/vector3.hpp"
#include <array>
#include <cassert>
#include <initializer_list>

namespace omath::collision
{

    // Minimal structural contract for the vector type used by GJK.
    template<class V>
    concept GjkVector = requires(const V& a, const V& b) {
        { -a } -> std::same_as<V>;
        { a - b } -> std::same_as<V>;
        { a.cross(b) } -> std::same_as<V>;
        { a.point_to_same_direction(b) } -> std::same_as<bool>;
    };

    template<GjkVector VectorType = Vector3<float>>
    class Simplex final
    {
        std::array<VectorType, 4> m_points{};
        std::size_t m_size{0};

    public:
        static constexpr std::size_t capacity = 4;

        constexpr Simplex() = default;

        constexpr Simplex& operator=(std::initializer_list<VectorType> list) noexcept
        {
            assert(list.size() <= capacity && "Simplex can have at most 4 points");
            m_size = 0;
            for (const auto& p : list)
                m_points[m_size++] = p;
            return *this;
        }

        constexpr void push_front(const VectorType& p) noexcept
        {
            const std::size_t limit = (m_size < capacity) ? m_size : capacity - 1;
            for (std::size_t i = limit; i > 0; --i)
                m_points[i] = m_points[i - 1];
            m_points[0] = p;
            if (m_size < capacity)
                ++m_size;
        }

        [[nodiscard]]
        constexpr const VectorType& operator[](std::size_t i) const noexcept
        {
            return m_points[i];
        }

        [[nodiscard]]
        constexpr VectorType& operator[](std::size_t i) noexcept
        {
            return m_points[i];
        }

        [[nodiscard]] constexpr std::size_t size() const noexcept
        {
            return m_size;
        }

        [[nodiscard]] constexpr bool empty() const noexcept
        {
            return m_size == 0;
        }

        [[nodiscard]] constexpr const VectorType& front() const noexcept
        {
            return m_points[0];
        }

        [[nodiscard]] constexpr const VectorType& back() const noexcept
        {
            return m_points[m_size - 1];
        }

        [[nodiscard]] constexpr const VectorType* data() const noexcept
        {
            return m_points.data();
        }

        [[nodiscard]] constexpr auto begin() const noexcept
        {
            return m_points.begin();
        }

        [[nodiscard]] constexpr auto end() const noexcept
        {
            return m_points.begin() + m_size;
        }

        constexpr void clear() noexcept
        {
            m_size = 0;
        }

        // GJK step: updates simplex + next search direction.
        // Returns true iff the origin lies inside the tetrahedron.
        [[nodiscard]] constexpr bool handle(VectorType& direction) noexcept
        {
            switch (m_size)
            {
            case 0:
                return false;
            case 1:
                return handle_point(direction);
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
        [[nodiscard]] constexpr bool handle_point(VectorType& direction) noexcept
        {
            const auto& a = m_points[0];
            direction = -a;
            return false;
        }

        template<class V>
        [[nodiscard]]
        static constexpr bool near_zero(const V& v, const float eps = 1e-7f)
        {
            return v.dot(v) <= eps * eps;
        }

        template<class V>
        [[nodiscard]]
        static constexpr V any_perp(const V& v)
        {
            for (const auto& dir : {V{1, 0, 0}, {0, 1, 0}, {0, 0, 1}})
                if (const auto d = v.cross(dir); !near_zero(d))
                    return d;
            std::unreachable();
        }

        [[nodiscard]]
        constexpr bool handle_line(VectorType& direction)
        {
            const auto& a = m_points[0];
            const auto& b = m_points[1];

            const auto ab = b - a;
            const auto ao = -a;

            if (ab.point_to_same_direction(ao))
            {
                // ReSharper disable once CppTooWideScopeInitStatement
                auto n = ab.cross(ao); // Needed to valid handle collision if colliders placed at same origin pos
                if (near_zero(n))
                {
                    // collinear: origin lies on ray AB (often on segment), pick any perp to escape
                    direction = any_perp(ab);
                }
                else
                {
                    direction = n.cross(ab);
                }
            }
            else
            {
                *this = {a};
                direction = ao;
            }
            return false;
        }

        [[nodiscard]] constexpr bool handle_triangle(VectorType& direction) noexcept
        {
            const auto& a = m_points[0];
            const auto& b = m_points[1];
            const auto& c = m_points[2];

            const auto ab = b - a;
            const auto ac = c - a;
            const auto ao = -a;

            const auto abc = ab.cross(ac);

            // Region AC
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

            // Region AB
            if (ab.cross(abc).point_to_same_direction(ao))
            {
                *this = {a, b};
                return handle_line(direction);
            }

            // Above or below triangle
            if (abc.point_to_same_direction(ao))
            {
                direction = abc;
            }
            else
            {
                *this = {a, c, b}; // flip winding
                direction = -abc;
            }
            return false;
        }

        [[nodiscard]] constexpr bool handle_tetrahedron(VectorType& direction) noexcept
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
            // Origin inside tetrahedron
            return true;
        }
    };

} // namespace omath::collision
