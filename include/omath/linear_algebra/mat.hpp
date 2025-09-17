//
// Created by vlad on 9/29/2024.
//
#pragma once
#include "vector3.hpp"
#include <algorithm>
#include <array>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <utility>


namespace omath
{
    struct MatSize
    {
        size_t rows, columns;
    };

    enum class MatStoreType : uint8_t
    {
        ROW_MAJOR = 0,
        COLUMN_MAJOR
    };

    template<typename M1, typename M2> concept MatTemplateEqual
            = (M1::rows == M2::rows) && (M1::columns == M2::columns)
              && std::is_same_v<typename M1::value_type, typename M2::value_type> && (M1::store_type == M2::store_type);

    template<size_t Rows = 0, size_t Columns = 0, class Type = float, MatStoreType StoreType = MatStoreType::ROW_MAJOR>
    requires std::is_arithmetic_v<Type>
    class Mat final
    {
    public:
        constexpr Mat() noexcept
        {
            clear();
        }

        [[nodiscard]]
        constexpr static MatStoreType get_store_ordering() noexcept
        {
            return StoreType;
        }
        constexpr Mat(const std::initializer_list<std::initializer_list<Type>>& rows)
        {
            if (rows.size() != Rows)
                throw std::invalid_argument("Initializer list rows size does not match template parameter Rows");

            auto row_it = rows.begin();
            for (size_t i = 0; i < Rows; ++i, ++row_it)
            {
                if (row_it->size() != Columns)
                    throw std::invalid_argument(
                            "All rows must have the same number of columns as template parameter Columns");

                auto col_it = row_it->begin();
                for (size_t j = 0; j < Columns; ++j, ++col_it)
                {
                    at(i, j) = std::move(*col_it);
                }
            }
        }

        constexpr explicit Mat(const Type* raw_data)
        {
            std::copy_n(raw_data, Rows * Columns, m_data.begin());
        }

        constexpr Mat(const Mat& other) noexcept
        {
            m_data = other.m_data;
        }

        [[nodiscard]]
        constexpr Type& operator[](const size_t row, const size_t col)
        {
            return at(row, col);
        }

        [[nodiscard]]
        constexpr const Type& operator[](const size_t row, const size_t col) const
        {
            return at(row, col);
        }

        constexpr Mat(Mat&& other) noexcept
        {
            m_data = std::move(other.m_data);
        }

        [[nodiscard]]
        static constexpr size_t row_count() noexcept
        {
            return Rows;
        }

        [[nodiscard]]
        static constexpr size_t columns_count() noexcept
        {
            return Columns;
        }

        [[nodiscard]]
        static consteval MatSize size() noexcept
        {
            return {Rows, Columns};
        }

        [[nodiscard]]
        constexpr const Type& at(const size_t row_index, const size_t column_index) const
        {
#if !defined(NDEBUG) && defined(OMATH_SUPRESS_SAFETY_CHECKS)
            if (row_index >= Rows || column_index >= Columns)
                throw std::out_of_range("Index out of range");
#endif
            if constexpr (StoreType == MatStoreType::ROW_MAJOR)
                return m_data[row_index * Columns + column_index];

            else if constexpr (StoreType == MatStoreType::COLUMN_MAJOR)
                return m_data[row_index + column_index * Rows];

            else
            {
                static_assert(false, "Invalid matrix access convention");
                std::unreachable();
            }
        }

        [[nodiscard]] constexpr Type& at(const size_t row_index, const size_t column_index)
        {
            return const_cast<Type&>(std::as_const(*this).at(row_index, column_index));
        }

        [[nodiscard]]
        constexpr Type sum() const noexcept
        {
            return std::accumulate(m_data.begin(), m_data.end(), static_cast<Type>(0));
        }

        constexpr void clear() noexcept
        {
            set(static_cast<Type>(0));
        }

        constexpr void set(const Type& value) noexcept
        {
            std::ranges::fill(m_data, value);
        }

        // Operator overloading for multiplication with another Mat
        template<size_t OtherColumns> [[nodiscard]]
        constexpr Mat<Rows, OtherColumns, Type, StoreType>
        operator*(const Mat<Columns, OtherColumns, Type, StoreType>& other) const
        {
            Mat<Rows, OtherColumns, Type, StoreType> result;

            if constexpr (StoreType == MatStoreType::ROW_MAJOR)
                return cache_friendly_multiply_row_major(other);
            if constexpr (StoreType == MatStoreType::COLUMN_MAJOR)
                return cache_friendly_multiply_col_major(other);
            std::unreachable();
        }

        constexpr Mat& operator*=(const Type& f) noexcept
        {
            std::ranges::for_each(m_data, [&f](auto& val) { val *= f; });
            return *this;
        }

        template<size_t OtherColumns> constexpr Mat<Rows, OtherColumns, Type, StoreType>
        operator*=(const Mat<Columns, OtherColumns, Type, StoreType>& other)
        {
            return *this = *this * other;
        }

        [[nodiscard]]
        constexpr Mat operator*(const Type& value) const noexcept
        {
            Mat result(*this);
            result *= value;
            return result;
        }

        constexpr Mat& operator/=(const Type& value) noexcept
        {
            std::ranges::for_each(m_data, [&value](auto& val) { val /= value; });
            return *this;
        }

        [[nodiscard]]
        constexpr Mat operator/(const Type& value) const noexcept
        {
            Mat result(*this);
            result /= value;
            return result;
        }

        constexpr Mat& operator=(const Mat& other) noexcept
        {
            if (this != &other)
                m_data = other.m_data;

            return *this;
        }

        constexpr Mat& operator=(Mat&& other) noexcept
        {
            if (this != &other)
                m_data = std::move(other.m_data);

            return *this;
        }

        [[nodiscard]]
        constexpr Mat<Columns, Rows, Type, StoreType> transposed() const noexcept
        {
            Mat<Columns, Rows, Type, StoreType> transposed;
            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < Columns; ++j)
                    transposed.at(j, i) = at(i, j);

            return transposed;
        }

        [[nodiscard]]
        constexpr Type determinant() const
        {
            static_assert(Rows == Columns, "Determinant is only defined for square matrices.");

            if constexpr (Rows == 1)
                return at(0, 0);

            if constexpr (Rows == 2)
                return at(0, 0) * at(1, 1) - at(0, 1) * at(1, 0);

            if constexpr (Rows > 2)
            {
                Type det = 0;
                for (size_t column = 0; column < Columns; ++column)
                {
                    const Type cofactor = at(0, column) * alg_complement(0, column);
                    det += cofactor;
                }
                return det;
            }
            std::unreachable();
        }

        [[nodiscard]]
        constexpr Mat<Rows - 1, Columns - 1, Type, StoreType> strip(const size_t row, const size_t column) const
        {
            static_assert(Rows - 1 > 0 && Columns - 1 > 0);
            Mat<Rows - 1, Columns - 1, Type, StoreType> result;
            for (size_t i = 0, m = 0; i < Rows; ++i)
            {
                if (i == row)
                    continue;
                for (size_t j = 0, n = 0; j < Columns; ++j)
                {
                    if (j == column)
                        continue;
                    result.at(m, n) = at(i, j);
                    ++n;
                }
                ++m;
            }
            return result;
        }

        [[nodiscard]]
        constexpr Type minor(const size_t row, const size_t column) const
        {
            return strip(row, column).determinant();
        }

        [[nodiscard]]
        constexpr Type alg_complement(const size_t row, const size_t column) const
        {
            const auto minor_value = minor(row, column);
            return (row + column + 2) % 2 == 0 ? minor_value : -minor_value;
        }

        [[nodiscard]]
        constexpr const std::array<Type, Rows * Columns>& raw_array() const
        {
            return m_data;
        }

        [[nodiscard]]
        constexpr std::array<Type, Rows * Columns>& raw_array()
        {
            return m_data;
        }

        [[nodiscard]]
        std::string to_string() const noexcept
        {
            std::ostringstream oss;
            oss << "[[";

            for (size_t i = 0; i < Rows; ++i)
            {
                if (i > 0)
                    oss << " [";

                for (size_t j = 0; j < Columns; ++j)
                {
                    oss << std::setw(9) << std::fixed << std::setprecision(3) << at(i, j);
                    if (j != Columns - 1)
                        oss << ", ";
                }
                oss << (i == Rows - 1 ? "]]" : "]\n");
            }
            return oss.str();
        }

        [[nodiscard]]
        bool operator==(const Mat& mat) const
        {
            return m_data == mat.m_data;
        }

        [[nodiscard]]
        bool operator!=(const Mat& mat) const
        {
            return !operator==(mat);
        }

        // Static methods that return fixed-size matrices
        [[nodiscard]]
        constexpr static Mat<4, 4> to_screen_mat(const Type& screen_width, const Type& screen_height) noexcept
        {
            return {
                    {screen_width / 2, 0, 0, 0},
                    {0, -screen_height / 2, 0, 0},
                    {0, 0, 1, 0},
                    {screen_width / 2, screen_height / 2, 0, 1},
            };
        }

        [[nodiscard]]
        constexpr std::optional<Mat> inverted() const
        {
            const auto det = determinant();

            if (det == 0)
                return std::nullopt;

            const auto transposed_mat = transposed();
            Mat result;

            for (std::size_t row = 0; row < Rows; row++)
                for (std::size_t column = 0; column < Rows; column++)
                    result.at(row, column) = transposed_mat.alg_complement(row, column);

            result /= det;

            return {result};
        }

    private:
        std::array<Type, Rows * Columns> m_data;

        template<size_t OtherColumns> [[nodiscard]]
        constexpr Mat<Rows, OtherColumns, Type, MatStoreType::ROW_MAJOR>
        cache_friendly_multiply_row_major(const Mat<Columns, OtherColumns, Type, MatStoreType::ROW_MAJOR>& other) const
        {
            Mat<Rows, OtherColumns, Type, MatStoreType::ROW_MAJOR> result;
            for (std::size_t i = 0; i < Rows; ++i)
                for (std::size_t k = 0; k < Columns; ++k)
                {
                    const Type aik = at(i, k);
                    for (std::size_t j = 0; j < OtherColumns; ++j)
                        result.at(i, j) += aik * other.at(k, j);
                }
            return result;
        }

        template<size_t OtherColumns> [[nodiscard]]
        constexpr Mat<Rows, OtherColumns, Type, MatStoreType::COLUMN_MAJOR> cache_friendly_multiply_col_major(
                const Mat<Columns, OtherColumns, Type, MatStoreType::COLUMN_MAJOR>& other) const
        {
            Mat<Rows, OtherColumns, Type, MatStoreType::COLUMN_MAJOR> result;
            for (std::size_t j = 0; j < OtherColumns; ++j)
                for (std::size_t k = 0; k < Columns; ++k)
                {
                    const Type bkj = other.at(k, j);
                    for (std::size_t i = 0; i < Rows; ++i)
                        result.at(i, j) += at(i, k) * bkj;
                }
            return result;
        }
    };

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR> [[nodiscard]]
    constexpr static Mat<1, 4, Type, St> mat_row_from_vector(const Vector3<Type>& vector) noexcept
    {
        return {{vector.x, vector.y, vector.z, 1}};
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR> [[nodiscard]]
    constexpr static Mat<4, 1, Type, St> mat_column_from_vector(const Vector3<Type>& vector) noexcept
    {
        return {{vector.x}, {vector.y}, {vector.z}, {1}};
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    [[nodiscard]]
    constexpr Mat<4, 4, Type, St> mat_translation(const Vector3<Type>& diff) noexcept
    {
        return
        {
                {1, 0, 0, diff.x},
                {0, 1, 0, diff.y},
                {0, 0, 1, diff.z},
                {0, 0, 0, 1},
        };
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR, class Angle>
    [[nodiscard]]
    Mat<4, 4, Type, St> mat_rotation_axis_x(const Angle& angle) noexcept
    {
        return
        {
            {1, 0,           0,            0},
            {0, angle.cos(), -angle.sin(), 0},
            {0, angle.sin(), angle.cos(),  0},
            {0, 0,           0,            1}
        };
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR, class Angle>
    [[nodiscard]]
    Mat<4, 4, Type, St> mat_rotation_axis_y(const Angle& angle) noexcept
    {
        return
        {
            {angle.cos(),  0, angle.sin(), 0},
            {0           , 1,           0, 0},
            {-angle.sin(), 0, angle.cos(), 0},
            {0           , 0,           0, 1}
        };
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR, class Angle>
    [[nodiscard]]
    Mat<4, 4, Type, St> mat_rotation_axis_z(const Angle& angle) noexcept
    {
        return
        {
            {angle.cos(), -angle.sin(), 0, 0},
            {angle.sin(),  angle.cos(), 0, 0},
            {          0,        0,     1, 0},
            {          0,        0,     0, 1},
        };
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    [[nodiscard]]
    static Mat<4, 4, Type, St> mat_camera_view(const Vector3<Type>& forward, const Vector3<Type>& right,
                                               const Vector3<Type>& up, const Vector3<Type>& camera_origin) noexcept
    {
        return  Mat<4, 4, Type, St>
        {
            {right.x,   right.y,   right.z,   0},
            {up.x,      up.y,      up.z,      0},
            {forward.x, forward.y, forward.z, 0},
            {0,         0,         0,         1},
        } * mat_translation<Type, St>(-camera_origin);
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    [[nodiscard]]
    Mat<4, 4, Type, St> mat_perspective_left_handed(const float field_of_view, const float aspect_ratio,
                                                    const float near, const float far) noexcept
    {
        const float fov_half_tan = std::tan(angles::degrees_to_radians(field_of_view) / 2.f);

        return {{1.f / (aspect_ratio * fov_half_tan), 0.f, 0.f, 0.f},
                {0.f, 1.f / fov_half_tan, 0.f, 0.f},
                {0.f, 0.f, (far + near) / (far - near), -(2.f * near * far) / (far - near)},
                {0.f, 0.f, 1.f, 0.f}};
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    [[nodiscard]]
    Mat<4, 4, Type, St> mat_perspective_right_handed(const float field_of_view, const float aspect_ratio,
                                                     const float near, const float far) noexcept
    {
        const float fov_half_tan = std::tan(angles::degrees_to_radians(field_of_view) / 2.f);

        return {{1.f / (aspect_ratio * fov_half_tan), 0.f, 0.f, 0.f},
                {0.f, 1.f / fov_half_tan, 0.f, 0.f},
                {0.f, 0.f, -(far + near) / (far - near), -(2.f * near * far) / (far - near)},
                {0.f, 0.f, -1.f, 0.f}};
    }
    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    [[nodiscard]]
    Mat<4, 4, Type, St> mat_ortho_left_handed(const Type left, const Type right,
                                              const Type bottom, const Type top,
                                              const Type near, const Type far) noexcept
    {
        return
        {
            { static_cast<Type>(2) / (right - left), 0.f,       0.f,    -(right + left) / (right - left)},
            { 0.f,      static_cast<Type>(2) / (top - bottom),  0.f,    -(top + bottom) / (top - bottom)},
            { 0.f,      0.f,       static_cast<Type>(2) / (far - near), -(far + near) / (far - near)    },
            { 0.f,      0.f,       0.f,                                 1.f                             }
        };
    }
    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    [[nodiscard]]
    Mat<4, 4, Type, St> mat_ortho_right_handed(const Type left, const Type right,
                                              const Type bottom, const Type top,
                                              const Type near, const Type far) noexcept
    {
        return
        {
                { static_cast<Type>(2) / (right - left), 0.f,       0.f,     -(right + left) / (right - left)},
                { 0.f,      static_cast<Type>(2) / (top - bottom),  0.f,     -(top + bottom) / (top - bottom)},
                { 0.f,      0.f,       -static_cast<Type>(2) / (far - near), -(far + near) / (far - near)    },
                { 0.f,      0.f,       0.f,                                  1.f                             }
        };
    }

} // namespace omath

template<size_t Rows, size_t Columns, class Type, omath::MatStoreType StoreType>
struct std::formatter<omath::Mat<Rows, Columns, Type, StoreType>> // NOLINT(*-dcl58-cpp)
{
    using MatType = omath::Mat<Rows, Columns, Type, StoreType>;
    [[nodiscard]]
    static constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }
    [[nodiscard]]
    static auto format(const MatType& mat, std::format_context& ctx)
    {
        return std::format_to(ctx.out(), "{}", mat.to_string());
    }
};