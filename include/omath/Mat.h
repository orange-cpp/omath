//
// Created by vlad on 9/29/2024.
//
#pragma once
#include <array>
#include <sstream>
#include <utility>
#include "Vector3.h"
#include <stdexcept>
#include "Angles.h"


namespace omath
{
    template <size_t Rows, size_t Columns>
    class Mat final
    {
    public:

        constexpr Mat()
        {
            Clear();
        }


        constexpr Mat(const std::initializer_list<std::initializer_list<float>>& rows)
        {
            if (rows.size() != Rows)
                throw std::invalid_argument("Initializer list rows size does not match template parameter Rows");

            auto rowIt = rows.begin();
            for (size_t i = 0; i < Rows; ++i, ++rowIt)
            {
                if (rowIt->size() != Columns)
                    throw std::invalid_argument("All rows must have the same number of columns as template parameter Columns");

                auto colIt = rowIt->begin();
                for (size_t j = 0; j < Columns; ++j, ++colIt)
                {
                    At(i, j) = *colIt;
                }
            }
        }


        constexpr Mat(const Mat& other)
        {
            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < Columns; ++j)
                    At(i, j) = other.At(i, j);
        }


        constexpr Mat(Mat&& other) noexcept
        {
            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < Columns; ++j)
                    At(i, j)  = other.At(i, j) ;
        }

        [[nodiscard]]
        static constexpr size_t RowCount() noexcept { return Rows; }

        [[nodiscard]]
        static constexpr size_t ColumnsCount() noexcept { return Columns; }

        [[nodiscard]]
        constexpr std::pair<size_t, size_t> Size() const noexcept { return { Rows, Columns }; }


        [[nodiscard]] constexpr const float& At(const size_t rowIndex, const size_t columnIndex) const
        {
            if (rowIndex >= Rows || columnIndex >= Columns)
                throw std::out_of_range("Index out of range");

            return m_data[rowIndex * Columns + columnIndex];
        }
        [[nodiscard]] constexpr float& At(const size_t rowIndex, const size_t columnIndex)
        {
            return const_cast<float&>(std::as_const(*this).At(rowIndex, columnIndex));
        }
        [[nodiscard]]
        constexpr float Sum() const
        {
            float sum = 0.f;
            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < Columns; ++j)
                    sum += At(i, j);

            return sum;
        }

        constexpr void Clear()
        {
            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < Columns; ++j)
                    At(i, j) = 0.f;
        }

        // Operator overloading for multiplication with another Mat
        template <size_t OtherColumns>
        constexpr Mat<Rows, OtherColumns> operator*(const Mat<Columns, OtherColumns>& other) const
        {
            Mat<Rows, OtherColumns> result;

            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < OtherColumns; ++j)
                {
                    float sum = 0.f;
                    for (size_t k = 0; k < Columns; ++k)
                        sum += At(i, k) * other.At(k, j);
                    result.At(i, j) = sum;
                }
            return result;
        }

        constexpr Mat& operator*=(float f)
        {
            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < Columns; ++j)
                    At(i, j) *= f;
            return *this;
        }

        constexpr Mat operator*(float f) const
        {
            Mat result(*this);
            result *= f;
            return result;
        }

        constexpr Mat& operator/=(float f)
        {
            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < Columns; ++j)
                    At(i, j) /= f;
            return *this;
        }

        constexpr Mat operator/(float f) const
        {
            Mat result(*this);
            result /= f;
            return result;
        }

        constexpr Mat& operator=(const Mat& other)
        {
            if (this == &other)
                return *this;
            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < Columns; ++j)
                    At(i, j) = other.At(i, j);
            return *this;
        }

        constexpr Mat& operator=(Mat&& other) noexcept
        {
            if (this == &other)
                return *this;

            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < Columns; ++j)
                    At(i, j) = other.At(i, j);

            return *this;
        }

        [[nodiscard]]
        constexpr Mat<Columns, Rows> Transpose() const
        {
            Mat<Columns, Rows> transposed;
            for (size_t i = 0; i < Rows; ++i)
                for (size_t j = 0; j < Columns; ++j)
                    transposed.At(j, i) = At(i, j);

            return transposed;
        }

        [[nodiscard]]
        constexpr float Determinant() const
        {
            static_assert(Rows == Columns, "Determinant is only defined for square matrices.");

            if constexpr (Rows == 1)
                return At(0, 0);

            else if constexpr (Rows == 2)
                return At(0, 0) * At(1, 1) - At(0, 1) * At(1, 0);
            else
            {
                float det = 0.f;
                for (size_t i = 0; i < Columns; ++i)
                {
                    const float cofactor = (i % 2 == 0 ? 1.f : -1.f) * At(0, i) * Minor(0, i).Determinant();
                    det += cofactor;
                }
                return det;
            }
        }

        [[nodiscard]]
        constexpr Mat<Rows - 1, Columns - 1> Minor(size_t row, size_t column) const
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
        std::string ToString() const
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
        constexpr static Mat<4, 4> ToScreenMat(float screenWidth, float screenHeight)
        {
            Mat<4, 4> mat;
            mat.At(0, 0) = screenWidth / 2.f;
            mat.At(1, 1) = -screenHeight / 2.f;
            mat.At(2, 2) = 1.f;
            mat.At(3, 0) = screenWidth / 2.f;
            mat.At(3, 1) = screenHeight / 2.f;
            mat.At(3, 3) = 1.f;
            return mat;
        }

        [[nodiscard]]
        constexpr static Mat<4, 4> TranslationMat(const Vector3& diff)
        {
            Mat<4, 4> mat;
            mat.At(0, 0) = 1.f;
            mat.At(1, 1) = 1.f;
            mat.At(2, 2) = 1.f;
            mat.At(3, 3) = 1.f;
            mat.At(3, 0) = diff.x;
            mat.At(3, 1) = diff.y;
            mat.At(3, 2) = diff.z;
            return mat;
        }

        [[nodiscard]]
        constexpr static Mat<4, 4> OrientationMat(const Vector3& forward, const Vector3& right, const Vector3& up)
        {
            Mat<4, 4> mat;

            mat.At(0, 0) = right.x;
            mat.At(0, 1) = up.x;
            mat.At(0, 2) = forward.x;
            mat.At(1, 0) = right.y;
            mat.At(1, 1) = up.y;
            mat.At(1, 2) = forward.y;
            mat.At(2, 0) = right.z;
            mat.At(2, 1) = up.z;
            mat.At(2, 2) = forward.z;
            mat.At(3, 3) = 1.f;

            return mat;
        }

        [[nodiscard]]
        constexpr static Mat<4, 4> ProjectionMat(const float fieldOfView, const float aspectRatio, const float near, const float far)
        {
            Mat<4, 4> mat;
            const float fovHalfTan = std::tan(angles::DegreesToRadians(fieldOfView) / 2.f);

            mat.At(0, 0) = 1.f / (aspectRatio * fovHalfTan);
            mat.At(1, 1) = 1.f / fovHalfTan;
            mat.At(2, 2) = (far + near) / (far - near);
            mat.At(2, 3) = (2.f * near * far) / (far - near);
            mat.At(3, 2) = -1.f;

            return mat;
        }

    private:
        std::array<float, Rows*Columns> m_data;
    };
}