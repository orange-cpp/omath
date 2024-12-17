#pragma once
#include <initializer_list>
#include <memory>
#include <string>
#include "omath/omath_export.hpp"

namespace omath
{
    class Vector3;

    class Matrix final
    {
    public:
        OMATH_API Matrix();
        OMATH_API Matrix(size_t rows, size_t columns);

        OMATH_API Matrix(const std::initializer_list<std::initializer_list<float>>& rows);

        [[nodiscard]]
        OMATH_API static Matrix ToScreenMatrix(float screenWidth, float screenHeight);

        [[nodiscard]]
        OMATH_API static Matrix TranslationMatrix(const Vector3& diff);

        [[nodiscard]]
        OMATH_API static Matrix OrientationMatrix(const Vector3& forward, const Vector3& right, const Vector3& up);

        [[nodiscard]]
        OMATH_API static Matrix ProjectionMatrix(float fieldOfView, float aspectRatio, float near, float far);

        OMATH_API Matrix(const Matrix& other);

        OMATH_API Matrix(size_t rows, size_t columns, const float* pRaw);

        OMATH_API Matrix(Matrix&& other) noexcept;

        [[nodiscard]]
        OMATH_API size_t RowCount() const noexcept;

        [[nodiscard]]
        OMATH_API size_t ColumnsCount() const noexcept;

        [[nodiscard]]
        OMATH_API std::pair<size_t, size_t> Size() const noexcept;

        [[nodiscard]]
        OMATH_API float& At(size_t iRow, size_t iCol);

        [[nodiscard]]
        OMATH_API float Sum();

        OMATH_API void SetDataFromRaw(const float* pRawMatrix);

        [[nodiscard]]
        OMATH_API Matrix Transpose() const;

        OMATH_API void Set(float val);

        [[nodiscard]]
        OMATH_API const float& At(size_t iRow, size_t iCol) const;

        OMATH_API Matrix operator*(const Matrix& other) const;

        OMATH_API Matrix& operator*=(const Matrix& other);

        OMATH_API Matrix operator*(float f) const;

        OMATH_API Matrix& operator*=(float f);

        OMATH_API Matrix& operator/=(float f);

        OMATH_API void Clear();

        [[nodiscard]]
        OMATH_API Matrix Strip(size_t row, size_t column) const;

        [[nodiscard]]
        OMATH_API float Minor(size_t i, size_t j) const;

        [[nodiscard]]
        OMATH_API float AlgComplement(size_t i, size_t j) const;

        [[nodiscard]]
        OMATH_API float Determinant() const;

        [[nodiscard]]
        OMATH_API const float* Raw() const;

        OMATH_API Matrix& operator=(const Matrix& other);

        OMATH_API Matrix& operator=(Matrix&& other) noexcept;

        OMATH_API Matrix operator/(float f) const;

        [[nodiscard]]
        OMATH_API std::string ToString() const;

        OMATH_API ~Matrix();

    private:
        size_t m_rows;
        size_t m_columns;
        std::unique_ptr<float[]> m_data;
    };
} // namespace omath
