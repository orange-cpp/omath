//
// Created by vlad on 9/29/2024.
//
#pragma once
#include <algorithm>
#include <array>
#include <sstream>
#include <utility>
#include "Vector3.hpp"
#include <stdexcept>
#include "Angles.hpp"


namespace omath
{
    template<size_t Rows, size_t Columns, class Type = float>
    class Mat final
    {
    public:
        constexpr Mat()
        {
            Clear();
        }


        constexpr Mat(const std::initializer_list<std::initializer_list<Type> > &rows)
        {
            if (rows.size() != Rows)
                throw std::invalid_argument("Initializer list rows size does not match template parameter Rows");

            auto rowIt = rows.begin();
            for (size_t i = 0; i < Rows; ++i, ++rowIt)
            {
                if (rowIt->size() != Columns)
                    throw std::invalid_argument(
                        "All rows must have the same number of columns as template parameter Columns");

                auto colIt = rowIt->begin();
                for (size_t j = 0; j < Columns; ++j, ++colIt)
                {
                    At(i, j) = *colIt;
                }
            }
        }


        constexpr Mat(const Mat &other) noexcept
        {
            m_data = other.m_data;
        }


        constexpr Mat(Mat &&other) noexcept
        {
            m_data = std::move(other.m_data);
        }

        [[nodiscard]]
        static consteval size_t RowCount() noexcept { return Rows; }

        [[nodiscard]]
        static consteval size_t ColumnsCount() noexcept { return Columns; }

        [[nodiscard]]
        static consteval std::pair<size_t, size_t> Size() noexcept { return {Rows, Columns}; }


        [[nodiscard]] constexpr const Type &At(const size_t rowIndex, const size_t columnIndex) const
        {
            if (rowIndex >= Rows || columnIndex >= Columns)
                throw std::out_of_range("Index out of range");

            return m_data[rowIndex * Columns + columnIndex];
        }

        [[nodiscard]] constexpr Type &At(const size_t rowIndex, const size_t columnIndex)
        {
            return const_cast<Type &>(std::as_const(*this).At(rowIndex, columnIndex));
        }

        [[nodiscard]]
        constexpr Type Sum() const noexcept
        {
            Type sum = 0;
            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < Columns; ++j)
                    sum += At(i, j);

            return sum;
        }

        constexpr void Clear() noexcept
        {
            Set(0);
        }

        constexpr void Set(const Type &value) noexcept
        {
            std::ranges::fill(m_data, value);
        }

        // Operator overloading for multiplication with another Mat
        template<size_t OtherColumns>
        constexpr Mat<Rows, OtherColumns> operator*(const Mat<Columns, OtherColumns> &other) const
        {
            Mat<Rows, OtherColumns> result;

            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < OtherColumns; ++j)
                {
                    Type sum = 0;
                    for (size_t k = 0; k < Columns; ++k)
                        sum += At(i, k) * other.At(k, j);
                    result.At(i, j) = sum;
                }
            return result;
        }

        constexpr Mat &operator*=(Type f) noexcept
        {
            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < Columns; ++j)
                    At(i, j) *= f;
            return *this;
        }

        template<size_t OtherColumns>
        constexpr Mat<Rows, OtherColumns> operator*=(const Mat<Columns, OtherColumns> &other)
        {
            return *this = *this * other;
        }

        constexpr Mat operator*(const Type &f) const noexcept
        {
            Mat result(*this);
            result *= f;
            return result;
        }

        constexpr Mat &operator/=(const Type &f) noexcept
        {
            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < Columns; ++j)
                    At(i, j) /= f;
            return *this;
        }

        constexpr Mat operator/(const Type &f) const noexcept
        {
            Mat result(*this);
            result /= f;
            return result;
        }

        constexpr Mat &operator=(const Mat &other) noexcept
        {
            if (this == &other)
                return *this;
            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < Columns; ++j)
                    At(i, j) = other.At(i, j);
            return *this;
        }

        constexpr Mat &operator=(Mat &&other) noexcept
        {
            if (this == &other)
                return *this;

            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < Columns; ++j)
                    At(i, j) = other.At(i, j);

            return *this;
        }

        [[nodiscard]]
        constexpr Mat<Columns, Rows> Transpose() const noexcept
        {
            Mat<Columns, Rows> transposed;
            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < Columns; ++j)
                    transposed.At(j, i) = At(i, j);

            return transposed;
        }

        [[nodiscard]]
        constexpr Type Determinant() const
        {
            static_assert(Rows == Columns, "Determinant is only defined for square matrices.");

            if constexpr (Rows == 1)
                return At(0, 0);

            else if constexpr (Rows == 2)
                return At(0, 0) * At(1, 1) - At(0, 1) * At(1, 0);
            else
            {
                Type det = 0;
                for (size_t i = 0; i < Columns; ++i)
                {
                    const Type cofactor = (i % 2 == 0 ? 1 : -1) * At(0, i) * Minor(0, i).Determinant();
                    det += cofactor;
                }
                return det;
            }
        }

        [[nodiscard]]
        constexpr Mat<Rows - 1, Columns - 1> Minor(const size_t row, const size_t column) const
        {
            Mat<Rows - 1, Columns - 1> result;
            for (size_t i = 0, m = 0; i < Rows; ++i)
            {
                if (i == row)
                    continue;
                for (size_t j = 0, n = 0; j < Columns; ++j)
                {
                    if (j == column)
                        continue;
                    result.At(m, n) = At(i, j);
                    ++n;
                }
                ++m;
            }
            return result;
        }

        [[nodiscard]]
        std::string ToString() const noexcept
        {
            std::ostringstream oss;
            for (size_t i = 0; i < Rows; ++i)
            {
                for (size_t j = 0; j < Columns; ++j)
                {
                    oss << At(i, j);
                    if (j != Columns - 1)
                        oss << ' ';
                }
                oss << '\n';
            }
            return oss.str();
        }

        // Static methods that return fixed-size matrices
        [[nodiscard]]
        constexpr static Mat<4, 4> ToScreenMat(const Type &screenWidth, const Type &screenHeight) noexcept
        {
            return
            {
                {screenWidth / 2, 0, 0, 0},
                {0, -screenHeight / 2, 0, 0},
                {0, 0, 1, 0},
                {screenWidth / 2, screenHeight / 2, 0, 1},
            };
        }

        [[nodiscard]]
        constexpr static Mat<4, 4> TranslationMat(const Vector3 &diff) noexcept
        {
            return
            {
                {1, 0, 0, 0},
                {0, 1, 0, 0},
                {0, 0, 1, 0},
                {diff.x, diff.y, diff.z, 1},
            };
        }

        [[nodiscard]]
        constexpr static Mat<4, 4> OrientationMat(const Vector3 &forward, const Vector3 &right,
                                                  const Vector3 &up) noexcept
        {
            return
            {
                {right.x, up.x, forward.x, 0},
                {right.y, up.y, forward.y, 0},
                {right.z, up.z, forward.z, 0},
                {0, 0, 0, 1},
            };
        }

        [[nodiscard]]
        constexpr static Mat<4, 4> ProjectionMat(const Type &fieldOfView, const Type &aspectRatio,
                                                 const Type &near, const Type &far, const Type &lensZoom) noexcept
        {
            const Type &fovHalfTan = std::tan(angles::DegreesToRadians(fieldOfView) / 2);
            const Type &frustumHeight = far - near;

            return
            {
                {-1 / (aspectRatio * fovHalfTan) * lensZoom, 0, 0, 0},
                {0, -1 / fovHalfTan * lensZoom, 0, 0},
                {0, 0, -far / frustumHeight, -1},
                {0, 0, near * far / frustumHeight, 0}
            };
        }

    private:
        std::array<Type, Rows * Columns> m_data;
    };
}
