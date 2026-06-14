//
// Created by orange on 6/11/2026.
//
#pragma once

#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numbers>
#include <type_traits>

namespace omath::internal
{
    // Embedded subset of GCE-Math 1.18.0.
    //
    // Original project:
    //   GCE-Math: A C++ generalized constant expression-based math library
    //   Copyright 2016-2024 Keith O'Hara
    //   Licensed under the Apache License, Version 2.0.
    namespace math_detail
    {
        using uint_t = unsigned int;
        using llint_t = long long int;
        using ullint_t = unsigned long long int;

        template<class Type>
        using limits = std::numeric_limits<Type>;

        template<class Type>
        constexpr Type pi = std::numbers::pi_v<Type>;

        template<class Type>
        constexpr Type half_pi = std::numbers::pi_v<Type> / static_cast<Type>(2);

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type abs(const Type value) noexcept
        {
            return value == Type{0} ? Type{0} : value < Type{0} ? -value : value;
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr int sgn(const Type value) noexcept
        {
            return value > Type{0} ? 1 : value < Type{0} ? -1 : 0;
        }

        [[nodiscard]]
        constexpr bool is_odd(const llint_t value) noexcept
        {
            return (value & 1U) != 0;
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr bool is_nan(const Type value) noexcept
        {
            return value != value;
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr bool is_neginf(const Type value) noexcept
        {
            return value == -limits<Type>::infinity();
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr bool is_posinf(const Type value) noexcept
        {
            return value == limits<Type>::infinity();
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr bool is_inf(const Type value) noexcept
        {
            return is_neginf(value) || is_posinf(value);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr bool is_finite(const Type value) noexcept
        {
            return !is_nan(value) && !is_inf(value);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr bool any_nan(const Type x, const Type y) noexcept
        {
            return is_nan(x) || is_nan(y);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr bool any_inf(const Type x, const Type y) noexcept
        {
            return is_inf(x) || is_inf(y);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr bool all_finite(const Type x, const Type y) noexcept
        {
            return is_finite(x) && is_finite(y);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr bool signbit(const Type value) noexcept
        {
            if constexpr (std::is_same_v<Type, float>)
                return (std::bit_cast<std::uint32_t>(value) & 0x80000000U) != 0;
            else if constexpr (std::is_same_v<Type, double>)
                return (std::bit_cast<std::uint64_t>(value) & 0x8000000000000000ULL) != 0;
            else
                return value < Type{0};
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr bool neg_zero(const Type value) noexcept
        {
            return value == Type{0} && signbit(value);
        }

        template<class Type, class ExpType>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type pow_integral_compute(const Type base, const ExpType exp_term) noexcept;

        template<class Type, class ExpType>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type pow_integral_compute_recur(const Type base, const Type value, const ExpType exp_term) noexcept
        {
            return exp_term > ExpType{1}
                           ? is_odd(static_cast<llint_t>(exp_term))
                                     ? pow_integral_compute_recur(base * base, value * base, exp_term / ExpType{2})
                                     : pow_integral_compute_recur(base * base, value, exp_term / ExpType{2})
                   : exp_term == ExpType{1} ? value * base
                                            : value;
        }

        template<class Type, class ExpType>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type pow_integral_sgn_check(const Type base, const ExpType exp_term) noexcept
        {
            if constexpr (std::is_signed_v<ExpType>)
                return exp_term < ExpType{0} ? Type{1} / pow_integral_compute(base, -exp_term)
                                             : pow_integral_compute_recur(base, Type{1}, exp_term);
            else
                return pow_integral_compute_recur(base, Type{1}, exp_term);
        }

        template<class Type, class ExpType>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type pow_integral_compute(const Type base, const ExpType exp_term) noexcept
        {
            return exp_term == ExpType{3}               ? base * base * base
                   : exp_term == ExpType{2}             ? base * base
                   : exp_term == ExpType{1}             ? base
                   : exp_term == ExpType{0}             ? Type{1}
                   : exp_term == limits<ExpType>::min() ? Type{0}
                   : exp_term == limits<ExpType>::max() ? limits<Type>::infinity()
                                                        : pow_integral_sgn_check(base, exp_term);
        }

        template<class Type, class ExpType>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type pow_integral(const Type base, const ExpType exp_term) noexcept
        {
            return pow_integral_compute(base, exp_term);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type sqrt_recur(const Type x, const Type xn, const int count) noexcept
        {
            return abs(xn - x / xn) / (Type{1} + xn) < limits<Type>::min() ? xn
                   : count < 100 ? sqrt_recur(x, Type{0.5} * (xn + x / xn), count + 1)
                                 : xn;
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type sqrt_simplify(const Type x, const Type m_val) noexcept
        {
            return x > Type{1e+08}   ? sqrt_simplify(x / Type{1e+08}, Type{1e+04} * m_val)
                   : x > Type{1e+06} ? sqrt_simplify(x / Type{1e+06}, Type{1e+03} * m_val)
                   : x > Type{1e+04} ? sqrt_simplify(x / Type{1e+04}, Type{1e+02} * m_val)
                   : x > Type{100}   ? sqrt_simplify(x / Type{100}, Type{10} * m_val)
                   : x > Type{4}     ? sqrt_simplify(x / Type{4}, Type{2} * m_val)
                                     : m_val * sqrt_recur(x, x / Type{2}, 0);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type sqrt(const Type x) noexcept
        {
            return is_nan(x)                                ? limits<Type>::quiet_NaN()
                   : x < Type{0}                            ? limits<Type>::quiet_NaN()
                   : is_posinf(x)                           ? x
                   : limits<Type>::min() > abs(x)           ? Type{0}
                   : limits<Type>::min() > abs(Type{1} - x) ? x
                                                            : sqrt_simplify(x, Type{1});
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type floor_int(const Type x, const Type x_whole) noexcept
        {
            return x_whole - static_cast<Type>((x < Type{0}) && (x < x_whole));
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type floor_check_internal(const Type x) noexcept
        {
            if constexpr (std::is_same_v<Type, float>)
                return abs(x) >= 8388608.f ? x : floor_int(x, static_cast<float>(static_cast<int>(x)));
            else if constexpr (std::is_same_v<Type, double>)
                return abs(x) >= 4503599627370496. ? x : floor_int(x, static_cast<double>(static_cast<llint_t>(x)));
            else
                return abs(x) >= 9223372036854775808.l
                               ? x
                               : static_cast<long double>(static_cast<ullint_t>(abs(x))) * sgn(x);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type floor(const Type x) noexcept
        {
            return is_nan(x)                      ? limits<Type>::quiet_NaN()
                   : !is_finite(x)                ? x
                   : limits<Type>::min() > abs(x) ? x
                                                  : floor_check_internal(x);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type trunc_int(const Type x) noexcept
        {
            return static_cast<Type>(static_cast<llint_t>(x));
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type trunc_check_internal(const Type x) noexcept
        {
            if constexpr (std::is_same_v<Type, float>)
                return abs(x) >= 8388608.f ? x : trunc_int(x);
            else if constexpr (std::is_same_v<Type, double>)
                return abs(x) >= 4503599627370496. ? x : trunc_int(x);
            else
                return abs(x) >= 9223372036854775808.l
                               ? x
                               : static_cast<long double>(static_cast<ullint_t>(abs(x))) * sgn(x);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type trunc(const Type x) noexcept
        {
            return is_nan(x)                      ? limits<Type>::quiet_NaN()
                   : !is_finite(x)                ? x
                   : limits<Type>::min() > abs(x) ? x
                                                  : trunc_check_internal(x);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type fmod(const Type x, const Type y) noexcept
        {
            return any_nan(x, y) || !all_finite(x, y) || limits<Type>::min() > abs(y) ? limits<Type>::quiet_NaN()
                                                                                      : x - trunc(x / y) * y;
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type tan_series_exp_long(const Type z) noexcept
        {
            return -Type{1} / z
                   + (z / Type{3}
                      + (pow_integral(z, 3) / Type{45}
                         + (Type{2} * pow_integral(z, 5) / Type{945} + pow_integral(z, 7) / Type{4725})));
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type tan_series_exp(const Type x) noexcept
        {
            return limits<Type>::min() > abs(x - half_pi<Type>) ? Type{1.633124e+16}
                                                                : tan_series_exp_long(x - half_pi<Type>);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type tan_cf_recur(const Type xx, const int max_depth) noexcept
        {
            Type result = static_cast<Type>(2 * max_depth - 1);
            for (int depth = max_depth - 1; depth >= 1; --depth)
                result = static_cast<Type>(2 * depth - 1) - xx / result;

            return result;
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type tan_cf_main(const Type x) noexcept
        {
            return x > Type{1.55} && x < Type{1.60} ? tan_series_exp(x)
                   : x > Type{1.4}                  ? x / tan_cf_recur(x * x, 45)
                   : x > Type{1}                    ? x / tan_cf_recur(x * x, 35)
                                                    : x / tan_cf_recur(x * x, 25);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type tan_begin(const Type x, const int count = 0) noexcept
        {
            return x > pi<Type> ? count > 1 ? limits<Type>::quiet_NaN()
                                            : tan_begin(x - pi<Type> * floor(x / pi<Type>), count + 1)
                                : tan_cf_main(x);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type tan(const Type x) noexcept
        {
            return is_nan(x)                      ? limits<Type>::quiet_NaN()
                   : limits<Type>::min() > abs(x) ? Type{0}
                   : x < Type{0}                  ? -tan_begin(-x)
                                                  : tan_begin(x);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type sin(const Type x) noexcept
        {
            return is_nan(x)                                      ? limits<Type>::quiet_NaN()
                   : limits<Type>::min() > abs(x)                 ? Type{0}
                   : limits<Type>::min() > abs(x - half_pi<Type>) ? Type{1}
                   : limits<Type>::min() > abs(x + half_pi<Type>) ? -Type{1}
                   : limits<Type>::min() > abs(x - pi<Type>)      ? Type{0}
                   : limits<Type>::min() > abs(x + pi<Type>)
                           ? -Type{0}
                           : (Type{2} * tan(x / Type{2})) / (Type{1} + tan(x / Type{2}) * tan(x / Type{2}));
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type cos(const Type x) noexcept
        {
            return is_nan(x)                                      ? limits<Type>::quiet_NaN()
                   : limits<Type>::min() > abs(x)                 ? Type{1}
                   : limits<Type>::min() > abs(x - half_pi<Type>) ? Type{0}
                   : limits<Type>::min() > abs(x + half_pi<Type>) ? Type{0}
                   : limits<Type>::min() > abs(x - pi<Type>)      ? -Type{1}
                   : limits<Type>::min() > abs(x + pi<Type>)
                           ? -Type{1}
                           : (Type{1} - tan(x / Type{2}) * tan(x / Type{2}))
                                     / (Type{1} + tan(x / Type{2}) * tan(x / Type{2}));
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type atan_series_order_calc(const Type xx, const Type x_pow, const uint_t order) noexcept
        {
            return Type{1} / (static_cast<Type>((order - 1) * 4 - 1) * x_pow)
                   - Type{1} / (static_cast<Type>((order - 1) * 4 + 1) * x_pow * xx);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type atan_series_order(const Type x, const uint_t order_begin, const uint_t max_order) noexcept
        {
            if (max_order == 1)
                return half_pi<Type> - Type{1} / x;

            const Type xx = x * x;
            Type result = atan_series_order_calc(xx, pow_integral(x, 4 * max_order - 5), max_order);
            auto depth = max_order - 1;

            while (depth > order_begin)
            {
                result += atan_series_order_calc(xx, pow_integral(x, 4 * depth - 5), depth);
                --depth;
            }

            return result + half_pi<Type> - Type{1} / x;
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type atan_series_main(const Type x) noexcept
        {
            return x < Type{3}      ? atan_series_order(x, 1U, 10U)
                   : x < Type{4}    ? atan_series_order(x, 1U, 9U)
                   : x < Type{5}    ? atan_series_order(x, 1U, 8U)
                   : x < Type{7}    ? atan_series_order(x, 1U, 7U)
                   : x < Type{11}   ? atan_series_order(x, 1U, 6U)
                   : x < Type{25}   ? atan_series_order(x, 1U, 5U)
                   : x < Type{100}  ? atan_series_order(x, 1U, 4U)
                   : x < Type{1000} ? atan_series_order(x, 1U, 3U)
                                    : atan_series_order(x, 1U, 2U);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type atan_cf_recur(const Type xx, const uint_t depth_begin, const uint_t max_depth) noexcept
        {
            auto depth = max_depth - 1;
            Type result = static_cast<Type>(2 * (depth + 1) - 1);

            while (depth > depth_begin - 1)
            {
                result = static_cast<Type>(2 * depth - 1) + static_cast<Type>(depth * depth) * xx / result;
                --depth;
            }

            return result;
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type atan_cf_main(const Type x) noexcept
        {
            return x < Type{0.5}   ? x / atan_cf_recur(x * x, 1U, 15U)
                   : x < Type{1}   ? x / atan_cf_recur(x * x, 1U, 25U)
                   : x < Type{1.5} ? x / atan_cf_recur(x * x, 1U, 35U)
                   : x < Type{2}   ? x / atan_cf_recur(x * x, 1U, 45U)
                                   : x / atan_cf_recur(x * x, 1U, 52U);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type atan_begin(const Type x) noexcept
        {
            return x > Type{2.5} ? atan_series_main(x) : atan_cf_main(x);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type atan(const Type x) noexcept
        {
            return is_nan(x)                      ? limits<Type>::quiet_NaN()
                   : limits<Type>::min() > abs(x) ? Type{0}
                   : x < Type{0}                  ? -atan_begin(-x)
                                                  : atan_begin(x);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type atan2(const Type y, const Type x) noexcept
        {
            return any_nan(y, x)                  ? limits<Type>::quiet_NaN()
                   : limits<Type>::min() > abs(x) ? limits<Type>::min() > abs(y)
                                                            ? neg_zero(y)   ? neg_zero(x) ? -pi<Type> : -Type{0}
                                                                : neg_zero(x) ? pi<Type>
                                                                            : Type{0}
                                                    : y > Type{0} ? half_pi<Type>
                                                                  : -half_pi<Type>
                   : x < Type{0} ? y < Type{0} ? atan(y / x) - pi<Type> : atan(y / x) + pi<Type>
                                 : atan(y / x);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type asin_compute(const Type x) noexcept
        {
            return x > Type{1}                              ? limits<Type>::quiet_NaN()
                   : limits<Type>::min() > abs(x - Type{1}) ? half_pi<Type>
                   : limits<Type>::min() > abs(x)           ? Type{0}
                                                            : atan(x / sqrt(Type{1} - x * x));
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type asin(const Type x) noexcept
        {
            return is_nan(x) ? limits<Type>::quiet_NaN() : x < Type{0} ? -asin_compute(-x) : asin_compute(x);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type acos_compute(const Type x) noexcept
        {
            return abs(x) > Type{1}                         ? limits<Type>::quiet_NaN()
                   : limits<Type>::min() > abs(x - Type{1}) ? Type{0}
                   : limits<Type>::min() > abs(x)           ? half_pi<Type>
                                                            : atan(sqrt(Type{1} - x * x) / x);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type acos(const Type x) noexcept
        {
            return is_nan(x) ? limits<Type>::quiet_NaN() : x > Type{0} ? acos_compute(x) : pi<Type> - acos_compute(-x);
        }

        template<class Type>
        requires std::is_floating_point_v<Type>
        [[nodiscard]]
        constexpr Type hypot(const Type x, const Type y) noexcept
        {
            return any_nan(x, y)                  ? limits<Type>::quiet_NaN()
                   : any_inf(x, y)                ? limits<Type>::infinity()
                   : limits<Type>::min() > abs(x) ? abs(y)
                   : limits<Type>::min() > abs(y) ? abs(x)
                                                  : abs(x) * sqrt(Type{1} + (y / x) * (y / x));
        }
    } // namespace math_detail

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]]
    constexpr Type sin(const Type& value) noexcept
    {
        if consteval
        {
            return math_detail::sin(value);
        }
        return std::sin(value);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]]
    constexpr Type cos(const Type& value) noexcept
    {
        if consteval
        {
            return math_detail::cos(value);
        }
        return std::cos(value);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]]
    constexpr Type tan(const Type& value) noexcept
    {
        if consteval
        {
            return math_detail::tan(value);
        }
        return std::tan(value);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]]
    constexpr Type atan(const Type& value) noexcept
    {
        if consteval
        {
            return math_detail::atan(value);
        }
        return std::atan(value);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]]
    constexpr Type atan2(const Type& y, const Type& x) noexcept
    {
        if consteval
        {
            return math_detail::atan2(y, x);
        }
        return std::atan2(y, x);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]]
    constexpr Type asin(const Type& value) noexcept
    {
        if consteval
        {
            return math_detail::asin(value);
        }
        return std::asin(value);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]]
    constexpr Type acos(const Type& value) noexcept
    {
        if consteval
        {
            return math_detail::acos(value);
        }
        return std::acos(value);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]]
    constexpr Type sqrt(const Type& value) noexcept
    {
        if consteval
        {
            return math_detail::sqrt(value);
        }
        return std::sqrt(value);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]]
    constexpr Type hypot(const Type& x, const Type& y) noexcept
    {
        if consteval
        {
            return math_detail::hypot(x, y);
        }
        return std::hypot(x, y);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]]
    constexpr Type hypot(const Type& x, const Type& y, const Type& z) noexcept
    {
        if consteval
        {
            return math_detail::sqrt(x * x + y * y + z * z);
        }
        return std::hypot(x, y, z);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]]
    constexpr Type abs(const Type& value) noexcept
    {
        if consteval
        {
            return math_detail::abs(value);
        }
        return std::abs(value);
    }

    template<class Type>
    requires std::is_floating_point_v<Type>
    [[nodiscard]]
    constexpr Type fmod(const Type& dividend, const Type& divisor) noexcept
    {
        if consteval
        {
            return math_detail::fmod(dividend, divisor);
        }
        return std::fmod(dividend, divisor);
    }
} // namespace omath::internal
