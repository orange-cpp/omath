//
// Created by vlad on 9/29/2024.
//
#pragma once
#include "omath/internal/constexpr_math.hpp"
#include "vector3.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

#ifdef OMATH_USE_AVX2
#include <immintrin.h>
#endif

#undef near
#undef far
// Undefine FreeBSD/BSD system macros that conflict with method names
#ifdef minor
#undef minor
#endif
#ifdef major
#undef major
#endif
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

    enum class NDCDepthRange : uint8_t
    {
        NEGATIVE_ONE_TO_ONE = 0, // OpenGL: [-1.0, 1.0]
        ZERO_TO_ONE // DirectX / Vulkan: [0.0, 1.0]
    };

    template<typename M1, typename M2> concept MatTemplateEqual =
            (M1::rows == M2::rows) && (M1::columns == M2::columns)
            && std::is_same_v<typename M1::value_type, typename M2::value_type> && (M1::store_type == M2::store_type);

    template<size_t Rows = 0, size_t Columns = 0, class Type = float, MatStoreType StoreType = MatStoreType::ROW_MAJOR>
    requires std::is_arithmetic_v<Type>
    class Mat final
    {
    public:
        using ContainedType = Type;
        constexpr Mat() noexcept
        {
            clear();
        }

        [[nodiscard("You must use store ordering")]]
        consteval static MatStoreType get_store_ordering() noexcept
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

        [[nodiscard("You must use element reference")]]
        constexpr Type& operator[](const size_t row, const size_t col)
        {
            return at(row, col);
        }

        [[nodiscard("You must use element reference")]]
        constexpr const Type& operator[](const size_t row, const size_t col) const
        {
            return at(row, col);
        }

        constexpr Mat(Mat&& other) noexcept
        {
            m_data = std::move(other.m_data);
        }

        [[nodiscard("You must use row count")]]
        static constexpr size_t row_count() noexcept
        {
            return Rows;
        }

        [[nodiscard("You must use column count")]]
        static constexpr size_t columns_count() noexcept
        {
            return Columns;
        }

        [[nodiscard("You must use matrix size")]]
        static constexpr MatSize size() noexcept
        {
            return {Rows, Columns};
        }

        [[nodiscard("You must use element reference")]]
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

        [[nodiscard("You must use element reference")]] constexpr Type& at(const size_t row_index,
                                                                           const size_t column_index)
        {
            return const_cast<Type&>(std::as_const(*this).at(row_index, column_index));
        }

        [[nodiscard("You must use sum of elements")]]
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
        template<size_t OtherColumns> [[nodiscard("You must use result matrix")]]
        constexpr Mat<Rows, OtherColumns, Type, StoreType>
        operator*(const Mat<Columns, OtherColumns, Type, StoreType>& other) const
        {
#ifdef OMATH_USE_AVX2
            if (std::is_constant_evaluated())
            {
                if constexpr (StoreType == MatStoreType::ROW_MAJOR)
                    return cache_friendly_multiply_row_major(other);
                else if constexpr (StoreType == MatStoreType::COLUMN_MAJOR)
                    return cache_friendly_multiply_col_major(other);
            }
            if constexpr (!std::is_same_v<Type, float> && !std::is_same_v<Type, double>)
            {
                if constexpr (StoreType == MatStoreType::ROW_MAJOR)
                    return cache_friendly_multiply_row_major(other);
                else if constexpr (StoreType == MatStoreType::COLUMN_MAJOR)
                    return cache_friendly_multiply_col_major(other);
            }
            else if constexpr (StoreType == MatStoreType::ROW_MAJOR)
                return avx_multiply_row_major(other);
            else if constexpr (StoreType == MatStoreType::COLUMN_MAJOR)
                return avx_multiply_col_major(other);
#else
            if constexpr (StoreType == MatStoreType::ROW_MAJOR)
                return cache_friendly_multiply_row_major(other);
            else if constexpr (StoreType == MatStoreType::COLUMN_MAJOR)
                return cache_friendly_multiply_col_major(other);
#endif
            else
                std::unreachable();
        }

        constexpr Mat& operator*=(const Type& f) noexcept
        {
            std::ranges::for_each(m_data,
                                  [&f](auto& val)
                                  {
                                      val *= f;
                                  });
            return *this;
        }

        template<size_t OtherColumns> constexpr Mat<Rows, OtherColumns, Type, StoreType>
        operator*=(const Mat<Columns, OtherColumns, Type, StoreType>& other)
        {
            return *this = *this * other;
        }

        [[nodiscard("You must use result matrix")]]
        constexpr Mat operator*(const Type& value) const noexcept
        {
            Mat result(*this);
            result *= value;
            return result;
        }

        constexpr Mat& operator/=(const Type& value) noexcept
        {
            std::ranges::for_each(m_data,
                                  [&value](auto& val)
                                  {
                                      val /= value;
                                  });
            return *this;
        }

        [[nodiscard("You must use result matrix")]]
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

        [[nodiscard("You must use transposed matrix")]]
        constexpr Mat<Columns, Rows, Type, StoreType> transposed() const noexcept
        {
            Mat<Columns, Rows, Type, StoreType> transposed;
            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < Columns; ++j)
                    transposed.at(j, i) = at(i, j);

            return transposed;
        }

        [[nodiscard("You must use determinant")]]
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
            else // For no reason MSVC triggers on it as unreachable code so we keep else here.
                std::unreachable();
        }

        [[nodiscard("You must use stripped matrix")]]
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

        [[nodiscard("You must use minor")]]
        constexpr Type minor(const size_t row, const size_t column) const
        {
            return strip(row, column).determinant();
        }

        [[nodiscard("You must use algebraic complement")]]
        constexpr Type alg_complement(const size_t row, const size_t column) const
        {
            const auto minor_value = minor(row, column);
            return (row + column + 2) % 2 == 0 ? minor_value : -minor_value;
        }

        [[nodiscard("You must use raw array")]]
        constexpr const std::array<Type, Rows * Columns>& raw_array() const
        {
            return m_data;
        }

        [[nodiscard("You must use raw array")]]
        constexpr std::array<Type, Rows * Columns>& raw_array()
        {
            return m_data;
        }

        [[nodiscard("You must use string representation")]]
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

        [[nodiscard("You must use wide string representation")]]
        std::wstring to_wstring() const noexcept
        {
            const auto ascii_string = to_string();
            return {ascii_string.cbegin(), ascii_string.cend()};
        }

        [[nodiscard("You must use UTF-8 string representation")]]
        // ReSharper disable once CppInconsistentNaming
        std::u8string to_u8string() const noexcept
        {
            const auto ascii_string = to_string();
            return {ascii_string.cbegin(), ascii_string.cend()};
        }

        [[nodiscard("You must use comparison result")]]
        bool operator==(const Mat& mat) const
        {
            return m_data == mat.m_data;
        }

        [[nodiscard("You must use comparison result")]]
        bool operator!=(const Mat& mat) const
        {
            return !operator==(mat);
        }

        // Static methods that return fixed-size matrices
        [[nodiscard("You must use screen matrix")]]
        constexpr static Mat<4, 4> to_screen_mat(const Type& screen_width, const Type& screen_height) noexcept
        {
            return {
                    {screen_width / 2, 0, 0, 0},
                    {0, -screen_height / 2, 0, 0},
                    {0, 0, 1, 0},
                    {screen_width / 2, screen_height / 2, 0, 1},
            };
        }

        [[nodiscard("You must use inverted matrix")]]
        constexpr std::optional<Mat> inverted() const
        {
            const auto det = determinant();

            if (internal::abs(det) < std::numeric_limits<Type>::epsilon())
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

        template<size_t OtherColumns> [[nodiscard("You must use result matrix")]]
        constexpr Mat<Rows, OtherColumns, Type, MatStoreType::ROW_MAJOR>
        cache_friendly_multiply_row_major(const Mat<Columns, OtherColumns, Type, MatStoreType::ROW_MAJOR>& other) const
        {
            Mat<Rows, OtherColumns, Type, MatStoreType::ROW_MAJOR> result;
            const Type* left_data = m_data.data();
            const Type* right_data = other.raw_array().data();
            Type* result_data = result.raw_array().data();

            for (std::size_t row_index = 0; row_index < Rows; ++row_index)
            {
                const Type* left_row = left_data + row_index * Columns;
                Type* result_row = result_data + row_index * OtherColumns;
                for (std::size_t column_index = 0; column_index < Columns; ++column_index)
                {
                    const Type current_number = left_row[column_index];
                    const Type* right_row = right_data + column_index * OtherColumns;
                    for (std::size_t other_column = 0; other_column < OtherColumns; ++other_column)
                        result_row[other_column] += current_number * right_row[other_column];
                }
            }
            return result;
        }

        template<size_t OtherColumns> [[nodiscard("You must use result matrix")]]
        constexpr Mat<Rows, OtherColumns, Type, MatStoreType::COLUMN_MAJOR> cache_friendly_multiply_col_major(
                const Mat<Columns, OtherColumns, Type, MatStoreType::COLUMN_MAJOR>& other) const
        {
            Mat<Rows, OtherColumns, Type, MatStoreType::COLUMN_MAJOR> result;
            const Type* left_data = m_data.data();
            const Type* right_data = other.raw_array().data();
            Type* result_data = result.raw_array().data();

            for (std::size_t other_column = 0; other_column < OtherColumns; ++other_column)
            {
                const Type* right_column = right_data + other_column * Columns;
                Type* result_column = result_data + other_column * Rows;
                for (std::size_t column_index = 0; column_index < Columns; ++column_index)
                {
                    const Type current_number = right_column[column_index];
                    const Type* left_column = left_data + column_index * Rows;
                    for (std::size_t row_index = 0; row_index < Rows; ++row_index)
                        result_column[row_index] += left_column[row_index] * current_number;
                }
            }
            return result;
        }
#ifdef OMATH_USE_AVX2
        template<size_t OtherColumns> [[nodiscard("You must use result matrix")]]
        constexpr Mat<Rows, OtherColumns, Type, MatStoreType::COLUMN_MAJOR>
        avx_multiply_col_major(const Mat<Columns, OtherColumns, Type, MatStoreType::COLUMN_MAJOR>& other) const
        {
            Mat<Rows, OtherColumns, Type, MatStoreType::COLUMN_MAJOR> result;

            const Type* this_mat_data = this->raw_array().data();
            const Type* other_mat_data = other.raw_array().data();
            Type* result_mat_data = result.raw_array().data();

            if constexpr (std::is_same_v<Type, float>)
            {
                constexpr std::size_t vector_size = 8;
                constexpr std::size_t block_size = vector_size * 4;
                for (std::size_t j = 0; j < OtherColumns; ++j)
                {
                    auto* c_col = reinterpret_cast<float*>(result_mat_data + j * Rows);
                    std::size_t i = 0;
                    for (; i + block_size <= Rows; i += block_size)
                    {
                        __m256 cvec0 = _mm256_setzero_ps();
                        __m256 cvec1 = _mm256_setzero_ps();
                        __m256 cvec2 = _mm256_setzero_ps();
                        __m256 cvec3 = _mm256_setzero_ps();
                        for (std::size_t k = 0; k < Columns; ++k)
                        {
                            const __m256 bkj_vec = _mm256_set1_ps(other_mat_data[k + j * Columns]);
                            const auto* a_col_k = this_mat_data + k * Rows + i;
                            cvec0 = _mm256_fmadd_ps(_mm256_loadu_ps(a_col_k), bkj_vec, cvec0);
                            cvec1 = _mm256_fmadd_ps(_mm256_loadu_ps(a_col_k + vector_size), bkj_vec, cvec1);
                            cvec2 = _mm256_fmadd_ps(_mm256_loadu_ps(a_col_k + vector_size * 2), bkj_vec, cvec2);
                            cvec3 = _mm256_fmadd_ps(_mm256_loadu_ps(a_col_k + vector_size * 3), bkj_vec, cvec3);
                        }
                        _mm256_storeu_ps(c_col + i, cvec0);
                        _mm256_storeu_ps(c_col + i + vector_size, cvec1);
                        _mm256_storeu_ps(c_col + i + vector_size * 2, cvec2);
                        _mm256_storeu_ps(c_col + i + vector_size * 3, cvec3);
                    }
                    for (; i + vector_size <= Rows; i += vector_size)
                    {
                        __m256 cvec = _mm256_setzero_ps();
                        for (std::size_t k = 0; k < Columns; ++k)
                        {
                            const __m256 bkj_vec = _mm256_set1_ps(other_mat_data[k + j * Columns]);
                            const auto* a_col_k = this_mat_data + k * Rows;
                            const __m256 a_vec = _mm256_loadu_ps(a_col_k + i);
                            cvec = _mm256_fmadd_ps(a_vec, bkj_vec, cvec);
                        }
                        _mm256_storeu_ps(c_col + i, cvec);
                    }
                    for (; i < Rows; ++i)
                        for (std::size_t k = 0; k < Columns; ++k)
                            c_col[i] += this_mat_data[i + k * Rows] * other_mat_data[k + j * Columns];
                }
            }
            else if (std::is_same_v<Type, double>)
            {
                constexpr std::size_t vector_size = 4;
                constexpr std::size_t block_size = vector_size * 4;
                for (std::size_t j = 0; j < OtherColumns; ++j)
                {
                    auto* c_col = reinterpret_cast<double*>(result_mat_data + j * Rows);
                    std::size_t i = 0;
                    for (; i + block_size <= Rows; i += block_size)
                    {
                        __m256d cvec0 = _mm256_setzero_pd();
                        __m256d cvec1 = _mm256_setzero_pd();
                        __m256d cvec2 = _mm256_setzero_pd();
                        __m256d cvec3 = _mm256_setzero_pd();
                        for (std::size_t k = 0; k < Columns; ++k)
                        {
                            const __m256d bkj_vec = _mm256_set1_pd(other_mat_data[k + j * Columns]);
                            const auto* a_col_k = this_mat_data + k * Rows + i;
                            cvec0 = _mm256_fmadd_pd(_mm256_loadu_pd(a_col_k), bkj_vec, cvec0);
                            cvec1 = _mm256_fmadd_pd(_mm256_loadu_pd(a_col_k + vector_size), bkj_vec, cvec1);
                            cvec2 = _mm256_fmadd_pd(_mm256_loadu_pd(a_col_k + vector_size * 2), bkj_vec, cvec2);
                            cvec3 = _mm256_fmadd_pd(_mm256_loadu_pd(a_col_k + vector_size * 3), bkj_vec, cvec3);
                        }
                        _mm256_storeu_pd(c_col + i, cvec0);
                        _mm256_storeu_pd(c_col + i + vector_size, cvec1);
                        _mm256_storeu_pd(c_col + i + vector_size * 2, cvec2);
                        _mm256_storeu_pd(c_col + i + vector_size * 3, cvec3);
                    }
                    for (; i + vector_size <= Rows; i += vector_size)
                    {
                        __m256d cvec = _mm256_setzero_pd();
                        for (std::size_t k = 0; k < Columns; ++k)
                        {
                            const __m256d bkj_vec = _mm256_set1_pd(other_mat_data[k + j * Columns]);
                            const auto* a_col_k = this_mat_data + k * Rows;
                            const __m256d a_vec = _mm256_loadu_pd(a_col_k + i);
                            cvec = _mm256_fmadd_pd(a_vec, bkj_vec, cvec);
                        }
                        _mm256_storeu_pd(c_col + i, cvec);
                    }
                    for (; i < Rows; ++i)
                        for (std::size_t k = 0; k < Columns; ++k)
                            c_col[i] += this_mat_data[i + k * Rows] * other_mat_data[k + j * Columns];
                }
            }
            else
                std::unreachable();

            return result;
        }

        template<size_t OtherColumns> [[nodiscard("You must use result matrix")]]
        constexpr Mat<Rows, OtherColumns, Type, MatStoreType::ROW_MAJOR>
        avx_multiply_row_major(const Mat<Columns, OtherColumns, Type, MatStoreType::ROW_MAJOR>& other) const
        {
            Mat<Rows, OtherColumns, Type, MatStoreType::ROW_MAJOR> result;

            const Type* this_mat_data = this->raw_array().data();
            const Type* other_mat_data = other.raw_array().data();
            Type* result_mat_data = result.raw_array().data();

            if constexpr (std::is_same_v<Type, float>)
            {
                constexpr std::size_t vector_size = 8;
                constexpr std::size_t block_size = vector_size * 4;
                for (std::size_t i = 0; i < Rows; ++i)
                {
                    auto* c_row = reinterpret_cast<float*>(result_mat_data + i * OtherColumns);
                    std::size_t j = 0;
                    for (; j + block_size <= OtherColumns; j += block_size)
                    {
                        __m256 cvec0 = _mm256_setzero_ps();
                        __m256 cvec1 = _mm256_setzero_ps();
                        __m256 cvec2 = _mm256_setzero_ps();
                        __m256 cvec3 = _mm256_setzero_ps();
                        for (std::size_t k = 0; k < Columns; ++k)
                        {
                            const __m256 aik_vec = _mm256_set1_ps(this_mat_data[i * Columns + k]);
                            const auto* b_row = other_mat_data + k * OtherColumns + j;
                            cvec0 = _mm256_fmadd_ps(_mm256_loadu_ps(b_row), aik_vec, cvec0);
                            cvec1 = _mm256_fmadd_ps(_mm256_loadu_ps(b_row + vector_size), aik_vec, cvec1);
                            cvec2 = _mm256_fmadd_ps(_mm256_loadu_ps(b_row + vector_size * 2), aik_vec, cvec2);
                            cvec3 = _mm256_fmadd_ps(_mm256_loadu_ps(b_row + vector_size * 3), aik_vec, cvec3);
                        }
                        _mm256_storeu_ps(c_row + j, cvec0);
                        _mm256_storeu_ps(c_row + j + vector_size, cvec1);
                        _mm256_storeu_ps(c_row + j + vector_size * 2, cvec2);
                        _mm256_storeu_ps(c_row + j + vector_size * 3, cvec3);
                    }
                    for (; j + vector_size <= OtherColumns; j += vector_size)
                    {
                        __m256 cvec = _mm256_setzero_ps();
                        for (std::size_t k = 0; k < Columns; ++k)
                        {
                            const __m256 aik_vec = _mm256_set1_ps(this_mat_data[i * Columns + k]);
                            const auto* b_row = other_mat_data + k * OtherColumns;
                            const __m256 b_vec = _mm256_loadu_ps(b_row + j);
                            cvec = _mm256_fmadd_ps(b_vec, aik_vec, cvec);
                        }
                        _mm256_storeu_ps(c_row + j, cvec);
                    }
                    for (; j < OtherColumns; ++j)
                        for (std::size_t k = 0; k < Columns; ++k)
                            c_row[j] += this_mat_data[i * Columns + k] * other_mat_data[k * OtherColumns + j];
                }
            }
            else if (std::is_same_v<Type, double>)
            {
                constexpr std::size_t vector_size = 4;
                constexpr std::size_t block_size = vector_size * 4;
                for (std::size_t i = 0; i < Rows; ++i)
                {
                    auto* c_row = reinterpret_cast<double*>(result_mat_data + i * OtherColumns);
                    std::size_t j = 0;
                    for (; j + block_size <= OtherColumns; j += block_size)
                    {
                        __m256d cvec0 = _mm256_setzero_pd();
                        __m256d cvec1 = _mm256_setzero_pd();
                        __m256d cvec2 = _mm256_setzero_pd();
                        __m256d cvec3 = _mm256_setzero_pd();
                        for (std::size_t k = 0; k < Columns; ++k)
                        {
                            const __m256d aik_vec = _mm256_set1_pd(this_mat_data[i * Columns + k]);
                            const auto* b_row = other_mat_data + k * OtherColumns + j;
                            cvec0 = _mm256_fmadd_pd(_mm256_loadu_pd(b_row), aik_vec, cvec0);
                            cvec1 = _mm256_fmadd_pd(_mm256_loadu_pd(b_row + vector_size), aik_vec, cvec1);
                            cvec2 = _mm256_fmadd_pd(_mm256_loadu_pd(b_row + vector_size * 2), aik_vec, cvec2);
                            cvec3 = _mm256_fmadd_pd(_mm256_loadu_pd(b_row + vector_size * 3), aik_vec, cvec3);
                        }
                        _mm256_storeu_pd(c_row + j, cvec0);
                        _mm256_storeu_pd(c_row + j + vector_size, cvec1);
                        _mm256_storeu_pd(c_row + j + vector_size * 2, cvec2);
                        _mm256_storeu_pd(c_row + j + vector_size * 3, cvec3);
                    }
                    for (; j + vector_size <= OtherColumns; j += vector_size)
                    {
                        __m256d cvec = _mm256_setzero_pd();
                        for (std::size_t k = 0; k < Columns; ++k)
                        {
                            const __m256d aik_vec = _mm256_set1_pd(this_mat_data[i * Columns + k]);
                            const auto* b_row = other_mat_data + k * OtherColumns;
                            const __m256d b_vec = _mm256_loadu_pd(b_row + j);
                            cvec = _mm256_fmadd_pd(b_vec, aik_vec, cvec);
                        }
                        _mm256_storeu_pd(c_row + j, cvec);
                    }
                    for (; j < OtherColumns; ++j)
                        for (std::size_t k = 0; k < Columns; ++k)
                            c_row[j] += this_mat_data[i * Columns + k] * other_mat_data[k * OtherColumns + j];
                }
            }
            else
                std::unreachable();
            return result;
        }
#endif
    };

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR> [[nodiscard("You must use row matrix")]]
    constexpr Mat<1, 4, Type, St> mat_row_from_vector(const Vector3<Type>& vector) noexcept
    {
        return {{vector.x, vector.y, vector.z, 1}};
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR> [[nodiscard("You must use column matrix")]]
    constexpr Mat<4, 1, Type, St> mat_column_from_vector(const Vector3<Type>& vector) noexcept
    {
        return {{vector.x}, {vector.y}, {vector.z}, {1}};
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    [[nodiscard("You must use translation matrix")]]
    constexpr Mat<4, 4, Type, St> mat_translation(const Vector3<Type>& diff) noexcept
    {
        return {
                {1, 0, 0, diff.x},
                {0, 1, 0, diff.y},
                {0, 0, 1, diff.z},
                {0, 0, 0, 1},
        };
    }
    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    [[nodiscard("You must use scale matrix")]]
    constexpr Mat<4, 4, Type, St> mat_scale(const Vector3<Type>& scale) noexcept
    {
        return {
                {scale.x, 0, 0, 0},
                {0, scale.y, 0, 0},
                {0, 0, scale.z, 0},
                {0, 0, 0, 1},
        };
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    [[nodiscard("You must use extracted origin")]]
    constexpr Vector3<Type> mat_extract_origin(const Mat<4, 4, Type, St>& mat) noexcept
    {
        return {mat.at(0, 3), mat.at(1, 3), mat.at(2, 3)};
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    [[nodiscard("You must use extracted scale")]]
    constexpr Vector3<Type> mat_extract_scale(const Mat<4, 4, Type, St>& mat) noexcept
    {
        auto column_length = [](const Type x, const Type y, const Type z)
        {
            return static_cast<Type>(internal::sqrt(x * x + y * y + z * z));
        };

        const auto scale_x = column_length(mat.at(0, 0), mat.at(1, 0), mat.at(2, 0));
        const auto scale_y = column_length(mat.at(0, 1), mat.at(1, 1), mat.at(2, 1));
        const auto scale_z = column_length(mat.at(0, 2), mat.at(1, 2), mat.at(2, 2));

        constexpr auto epsilon = std::numeric_limits<Type>::epsilon();

        return {
                internal::abs(scale_x) < epsilon ? Type{1} : scale_x,
                internal::abs(scale_y) < epsilon ? Type{1} : scale_y,
                internal::abs(scale_z) < epsilon ? Type{1} : scale_z,
        };
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    requires std::is_floating_point_v<Type>
    [[nodiscard("You must use extracted rotation")]]
    constexpr Vector3<Type> mat_extract_rotation_zyx(const Mat<4, 4, Type, St>& mat) noexcept
    {
        const auto scale = mat_extract_scale(mat);
        const auto m00 = mat.at(0, 0) / scale.x;
        const auto m10 = mat.at(1, 0) / scale.x;
        const auto m20 = mat.at(2, 0) / scale.x;
        const auto m21 = mat.at(2, 1) / scale.y;
        const auto m22 = mat.at(2, 2) / scale.z;

        return {
                angles::radians_to_degrees(internal::atan2(m21, m22)),
                angles::radians_to_degrees(internal::asin(std::clamp(-m20, Type{-1}, Type{1}))),
                angles::radians_to_degrees(internal::atan2(m10, m00)),
        };
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR, class Angle>
    [[nodiscard("You must use rotation matrix")]]
    constexpr Mat<4, 4, Type, St> mat_rotation_axis_x(const Angle& angle) noexcept
    {
        return {{1, 0, 0, 0}, {0, angle.cos(), -angle.sin(), 0}, {0, angle.sin(), angle.cos(), 0}, {0, 0, 0, 1}};
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR, class Angle>
    [[nodiscard("You must use rotation matrix")]]
    constexpr Mat<4, 4, Type, St> mat_rotation_axis_y(const Angle& angle) noexcept
    {
        return {{angle.cos(), 0, angle.sin(), 0}, {0, 1, 0, 0}, {-angle.sin(), 0, angle.cos(), 0}, {0, 0, 0, 1}};
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR, class Angle>
    [[nodiscard("You must use rotation matrix")]]
    constexpr Mat<4, 4, Type, St> mat_rotation_axis_z(const Angle& angle) noexcept
    {
        return {
                {angle.cos(), -angle.sin(), 0, 0},
                {angle.sin(), angle.cos(), 0, 0},
                {0, 0, 1, 0},
                {0, 0, 0, 1},
        };
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    [[nodiscard("You must use camera view matrix")]]
    constexpr Mat<4, 4, Type, St> mat_camera_view(const Vector3<Type>& forward, const Vector3<Type>& right,
                                                  const Vector3<Type>& up, const Vector3<Type>& camera_origin) noexcept
    {
        return Mat<4, 4, Type, St>{
                       {right.x, right.y, right.z, 0},
                       {up.x, up.y, up.z, 0},
                       {forward.x, forward.y, forward.z, 0},
                       {0, 0, 0, 1},
               }
               * mat_translation<Type, St>(-camera_origin);
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR,
             NDCDepthRange DepthRange = NDCDepthRange::NEGATIVE_ONE_TO_ONE>
    [[nodiscard("You must use perspective matrix")]] constexpr Mat<4, 4, Type, St>
    mat_perspective_left_handed_vertical_fov(const Type field_of_view, const Type aspect_ratio, const Type near,
                                             const Type far) noexcept
    {
        const auto fov_half_tan = internal::tan(angles::degrees_to_radians(field_of_view) / Type{2});
        if constexpr (DepthRange == NDCDepthRange::ZERO_TO_ONE)
            return {{Type{1} / (aspect_ratio * fov_half_tan), Type{0}, Type{0}, Type{0}},
                    {Type{0}, Type{1} / fov_half_tan, Type{0}, Type{0}},
                    {Type{0}, Type{0}, far / (far - near), -(near * far) / (far - near)},
                    {Type{0}, Type{0}, Type{1}, Type{0}}};
        else if constexpr (DepthRange == NDCDepthRange::NEGATIVE_ONE_TO_ONE)
            return {{Type{1} / (aspect_ratio * fov_half_tan), Type{0}, Type{0}, Type{0}},
                    {Type{0}, Type{1} / fov_half_tan, Type{0}, Type{0}},
                    {Type{0}, Type{0}, (far + near) / (far - near), -(Type{2} * near * far) / (far - near)},
                    {Type{0}, Type{0}, Type{1}, Type{0}}};
        else
            std::unreachable();
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR,
             NDCDepthRange DepthRange = NDCDepthRange::NEGATIVE_ONE_TO_ONE>
    [[nodiscard("You must use perspective matrix")]] constexpr Mat<4, 4, Type, St>
    mat_perspective_right_handed_vertical_fov(const Type field_of_view, const Type aspect_ratio, const Type near,
                                              const Type far) noexcept
    {
        const auto fov_half_tan = internal::tan(angles::degrees_to_radians(field_of_view) / Type{2});

        if constexpr (DepthRange == NDCDepthRange::ZERO_TO_ONE)
            return {{Type{1} / (aspect_ratio * fov_half_tan), Type{0}, Type{0}, Type{0}},
                    {Type{0}, Type{1} / fov_half_tan, Type{0}, Type{0}},
                    {Type{0}, Type{0}, -far / (far - near), -(near * far) / (far - near)},
                    {Type{0}, Type{0}, -Type{1}, Type{0}}};
        else if constexpr (DepthRange == NDCDepthRange::NEGATIVE_ONE_TO_ONE)
            return {{Type{1} / (aspect_ratio * fov_half_tan), Type{0}, Type{0}, Type{0}},
                    {Type{0}, Type{1} / fov_half_tan, Type{0}, Type{0}},
                    {Type{0}, Type{0}, -(far + near) / (far - near), -(Type{2} * near * far) / (far - near)},
                    {Type{0}, Type{0}, -Type{1}, Type{0}}};
        else
            std::unreachable();
    }

    // Horizontal-FOV variants — use these when the engine reports FOV as
    // horizontal (UE's FMinimalViewInfo::FOV, Quake-family fov_x, etc.).
    // X and Y scales derived as: X = 1 / tan(hfov/2), Y = aspect / tan(hfov/2).
    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR,
             NDCDepthRange DepthRange = NDCDepthRange::NEGATIVE_ONE_TO_ONE>
    [[nodiscard("You must use perspective matrix")]] constexpr Mat<4, 4, Type, St>
    mat_perspective_left_handed_horizontal_fov(const Type horizontal_fov, const Type aspect_ratio, const Type near,
                                               const Type far) noexcept
    {
        const auto inv_tan_half_hfov = Type{1} / internal::tan(angles::degrees_to_radians(horizontal_fov) / Type{2});
        const auto x_axis = inv_tan_half_hfov;
        const auto y_axis = inv_tan_half_hfov * aspect_ratio;

        if constexpr (DepthRange == NDCDepthRange::ZERO_TO_ONE)
            return {{x_axis, Type{0}, Type{0}, Type{0}},
                    {Type{0}, y_axis, Type{0}, Type{0}},
                    {Type{0}, Type{0}, far / (far - near), -(near * far) / (far - near)},
                    {Type{0}, Type{0}, Type{1}, Type{0}}};
        else if constexpr (DepthRange == NDCDepthRange::NEGATIVE_ONE_TO_ONE)
            return {{x_axis, Type{0}, Type{0}, Type{0}},
                    {Type{0}, y_axis, Type{0}, Type{0}},
                    {Type{0}, Type{0}, (far + near) / (far - near), -(Type{2} * near * far) / (far - near)},
                    {Type{0}, Type{0}, Type{1}, Type{0}}};
        else
            std::unreachable();
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR,
             NDCDepthRange DepthRange = NDCDepthRange::NEGATIVE_ONE_TO_ONE>
    [[nodiscard("You must use perspective matrix")]] constexpr Mat<4, 4, Type, St>
    mat_perspective_right_handed_horizontal_fov(const Type horizontal_fov, const Type aspect_ratio, const Type near,
                                                const Type far) noexcept
    {
        const auto inv_tan_half_hfov = Type{1} / internal::tan(angles::degrees_to_radians(horizontal_fov) / Type{2});

        const auto x_axis = inv_tan_half_hfov;
        const auto y_axis = inv_tan_half_hfov * aspect_ratio;

        if constexpr (DepthRange == NDCDepthRange::ZERO_TO_ONE)
            return {{x_axis, Type{0}, Type{0}, Type{0}},
                    {Type{0}, y_axis, Type{0}, Type{0}},
                    {Type{0}, Type{0}, -far / (far - near), -(near * far) / (far - near)},
                    {Type{0}, Type{0}, -Type{1}, Type{0}}};
        else if constexpr (DepthRange == NDCDepthRange::NEGATIVE_ONE_TO_ONE)
            return {{x_axis, Type{0}, Type{0}, Type{0}},
                    {Type{0}, y_axis, Type{0}, Type{0}},
                    {Type{0}, Type{0}, -(far + near) / (far - near), -(Type{2} * near * far) / (far - near)},
                    {Type{0}, Type{0}, -Type{1}, Type{0}}};
        else
            std::unreachable();
    }
    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR,
             NDCDepthRange DepthRange = NDCDepthRange::NEGATIVE_ONE_TO_ONE>
    [[nodiscard("You must use ortho matrix")]] constexpr Mat<4, 4, Type, St>
    mat_ortho_left_handed(const Type left, const Type right, const Type bottom, const Type top, const Type near,
                          const Type far) noexcept
    {
        if constexpr (DepthRange == NDCDepthRange::ZERO_TO_ONE)
            return {{static_cast<Type>(2) / (right - left), 0.f, 0.f, -(right + left) / (right - left)},
                    {0.f, static_cast<Type>(2) / (top - bottom), 0.f, -(top + bottom) / (top - bottom)},
                    {0.f, 0.f, static_cast<Type>(1) / (far - near), -near / (far - near)},
                    {0.f, 0.f, 0.f, 1.f}};
        else if constexpr (DepthRange == NDCDepthRange::NEGATIVE_ONE_TO_ONE)
            return {{static_cast<Type>(2) / (right - left), 0.f, 0.f, -(right + left) / (right - left)},
                    {0.f, static_cast<Type>(2) / (top - bottom), 0.f, -(top + bottom) / (top - bottom)},
                    {0.f, 0.f, static_cast<Type>(2) / (far - near), -(far + near) / (far - near)},
                    {0.f, 0.f, 0.f, 1.f}};
        else
            std::unreachable();
    }
    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR,
             NDCDepthRange DepthRange = NDCDepthRange::NEGATIVE_ONE_TO_ONE>
    [[nodiscard("You must use ortho matrix")]] constexpr Mat<4, 4, Type, St>
    mat_ortho_right_handed(const Type left, const Type right, const Type bottom, const Type top, const Type near,
                           const Type far) noexcept
    {
        if constexpr (DepthRange == NDCDepthRange::ZERO_TO_ONE)
            return {{static_cast<Type>(2) / (right - left), 0.f, 0.f, -(right + left) / (right - left)},
                    {0.f, static_cast<Type>(2) / (top - bottom), 0.f, -(top + bottom) / (top - bottom)},
                    {0.f, 0.f, -static_cast<Type>(1) / (far - near), -near / (far - near)},
                    {0.f, 0.f, 0.f, 1.f}};
        else if constexpr (DepthRange == NDCDepthRange::NEGATIVE_ONE_TO_ONE)
            return {{static_cast<Type>(2) / (right - left), 0.f, 0.f, -(right + left) / (right - left)},
                    {0.f, static_cast<Type>(2) / (top - bottom), 0.f, -(top + bottom) / (top - bottom)},
                    {0.f, 0.f, -static_cast<Type>(2) / (far - near), -(far + near) / (far - near)},
                    {0.f, 0.f, 0.f, 1.f}};
        else
            std::unreachable();
    }
    template<class T = float, MatStoreType St = MatStoreType::COLUMN_MAJOR>
    constexpr Mat<4, 4, T, St> mat_look_at_left_handed(const Vector3<T>& eye, const Vector3<T>& center,
                                                       const Vector3<T>& up)
    {
        const Vector3<T> f = (center - eye).normalized();
        const Vector3<T> s = f.cross(up).normalized();
        const Vector3<T> u = s.cross(f);
        return mat_camera_view<T, St>(f, s, u, eye);
    }

    template<class T = float, MatStoreType St = MatStoreType::COLUMN_MAJOR>
    constexpr Mat<4, 4, T, St> mat_look_at_right_handed(const Vector3<T>& eye, const Vector3<T>& center,
                                                        const Vector3<T>& up)
    {
        const Vector3<T> f = (center - eye).normalized();
        const Vector3<T> s = f.cross(up).normalized();
        const Vector3<T> u = s.cross(f);
        return mat_camera_view<T, St>(-f, s, u, eye);
    }

} // namespace omath

template<size_t Rows, size_t Columns, class Type, omath::MatStoreType StoreType>
struct std::formatter<omath::Mat<Rows, Columns, Type, StoreType>> final // NOLINT(*-dcl58-cpp)
{
    using MatType = omath::Mat<Rows, Columns, Type, StoreType>;
    [[nodiscard("You must use parse iterator")]]
    static constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template<class FormatContext>
    [[nodiscard("You must use format iterator")]]
    static auto format(const MatType& mat, FormatContext& ctx)
    {
        if constexpr (std::is_same_v<typename FormatContext::char_type, char>)
            return std::format_to(ctx.out(), "{}", mat.to_string());

        if constexpr (std::is_same_v<typename FormatContext::char_type, wchar_t>)
            return std::format_to(ctx.out(), L"{}", mat.to_wstring());

        if constexpr (std::is_same_v<typename FormatContext::char_type, char8_t>)
            return std::format_to(ctx.out(), u8"{}", mat.to_u8string());
    }
};
