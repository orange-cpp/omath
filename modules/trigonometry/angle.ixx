//
// Created by Vlad on 9/1/2025.
//
module;
#include <algorithm>
#include <cmath>
#include <format>
#include <numbers>
#include <utility>

export module omath.angle;

export namespace omath
{
    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]] constexpr Type radians_to_degrees(const Type& radians) noexcept
    {
        return radians * (static_cast<Type>(180) / std::numbers::pi_v<Type>);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]] constexpr Type degrees_to_radians(const Type& degrees) noexcept
    {
        return degrees * (std::numbers::pi_v<Type> / static_cast<Type>(180));
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]] Type horizontal_fov_to_vertical(const Type& horizontal_fov, const Type& aspect) noexcept
    {
        const auto fov_rad = degrees_to_radians(horizontal_fov);

        const auto vert_fov = static_cast<Type>(2) * std::atan(std::tan(fov_rad / static_cast<Type>(2)) / aspect);

        return radians_to_degrees(vert_fov);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]] Type vertical_fov_to_horizontal(const Type& vertical_fov, const Type& aspect) noexcept
    {
        const auto fov_as_radians = degrees_to_radians(vertical_fov);

        const auto horizontal_fov =
                static_cast<Type>(2) * std::atan(std::tan(fov_as_radians / static_cast<Type>(2)) * aspect);

        return radians_to_degrees(horizontal_fov);
    }

    template<class Type>
    requires std::is_arithmetic_v<Type>
    [[nodiscard]] Type wrap_angle(const Type& angle, const Type& min, const Type& max) noexcept
    {
        if (angle <= max && angle >= min)
            return angle;

        const Type range = max - min;

        Type wrapped_angle = std::fmod(angle - min, range);

        if (wrapped_angle < 0)
            wrapped_angle += range;

        return wrapped_angle + min;
    }

    enum class AngleFlags
    {
        Normalized = 0,
        Clamped = 1,
    };

    template<class Type = float, Type min = Type(0), Type max = Type(360), AngleFlags flags = AngleFlags::Normalized>
    requires std::is_arithmetic_v<Type>
    class Angle
    {
        Type m_angle;
        constexpr explicit Angle(const Type& degrees) noexcept
        {
            if constexpr (flags == AngleFlags::Normalized)
                m_angle = wrap_angle(degrees, min, max);

            else if constexpr (flags == AngleFlags::Clamped)
                m_angle = std::clamp(degrees, min, max);
            else
            {
                static_assert(false);
                std::unreachable();
            }
        }

    public:
        [[nodiscard]]
        constexpr static Angle from_degrees(const Type& degrees) noexcept
        {
            return Angle{degrees};
        }
        constexpr Angle() noexcept: m_angle(0)
        {
        }
        [[nodiscard]]
        constexpr static Angle from_radians(const Type& degrees) noexcept
        {
            return Angle{radians_to_degrees<Type>(degrees)};
        }

        [[nodiscard]]
        constexpr const Type& operator*() const noexcept
        {
            return m_angle;
        }

        [[nodiscard]]
        constexpr Type as_degrees() const noexcept
        {
            return m_angle;
        }

        [[nodiscard]]
        constexpr Type as_radians() const noexcept
        {
            return degrees_to_radians(m_angle);
        }

        [[nodiscard]]
        Type sin() const noexcept
        {
            return std::sin(as_radians());
        }

        [[nodiscard]]
        Type cos() const noexcept
        {
            return std::cos(as_radians());
        }

        [[nodiscard]]
        Type tan() const noexcept
        {
            return std::tan(as_radians());
        }

        [[nodiscard]]
        Type atan() const noexcept
        {
            return std::atan(as_radians());
        }

        [[nodiscard]]
        Type cot() const noexcept
        {
            return cos() / sin();
        }

        constexpr Angle& operator+=(const Angle& other) noexcept
        {
            if constexpr (flags == AngleFlags::Normalized)
                m_angle = wrap_angle(m_angle + other.m_angle, min, max);

            else if constexpr (flags == AngleFlags::Clamped)
                m_angle = std::clamp(m_angle + other.m_angle, min, max);
            else
            {
                static_assert(false);
                std::unreachable();
            }

            return *this;
        }

        [[nodiscard]]
        constexpr std::partial_ordering operator<=>(const Angle& other) const noexcept = default;

        constexpr Angle& operator-=(const Angle& other) noexcept
        {
            return operator+=(-other);
        }

        [[nodiscard]]
        constexpr Angle& operator+(const Angle& other) noexcept
        {
            if constexpr (flags == AngleFlags::Normalized)
                return {wrap_angle(m_angle + other.m_angle, min, max)};

            else if constexpr (flags == AngleFlags::Clamped)
                return {std::clamp(m_angle + other.m_angle, min, max)};

            else
                static_assert(false);

            std::unreachable();
        }

        [[nodiscard]]
        constexpr Angle& operator-(const Angle& other) noexcept
        {
            return operator+(-other);
        }

        [[nodiscard]]
        constexpr Angle operator-() const noexcept
        {
            return Angle{-m_angle};
        }
    };
} // namespace omath