#pragma once
#include <vector>
#include <memory>
#include <string>
#include <initializer_list>

namespace omath
{
    class Vector3;

    class Matrix final
    {
    public:
        Matrix(size_t rows, size_t columns);

        Matrix(const std::initializer_list<std::initializer_list<float>>& rows);

        [[nodiscard]]
        static Matrix ToScreenMatrix(float screenWidth, float screenHeight);

        [[nodiscard]]
        static Matrix TranslationMatrix(const Vector3& diff);

        [[nodiscard]]
        static Matrix OrientationMatrix(const Vector3& forward, const Vector3& right, const Vector3& up);

        [[nodiscard]]
        static Matrix ProjectionMatrix(float fielOfView, float aspectRatio,float near, float far);

        Matrix(const Matrix &other);

        Matrix(size_t rows, size_t columns, const float *pRaw);

        Matrix(Matrix &&other) noexcept;

        [[nodiscard]]
        size_t RowCount() const noexcept;

        [[nodiscard]]
        size_t ColumnsCount() const noexcept;

        [[nodiscard]]
        std::pair<size_t, size_t> Size() const noexcept;

        [[nodiscard]]
        float &At(size_t iRow, size_t iCol);

        [[nodiscard]]
        float Sum();

        void SetDataFromRaw(const float* pRawMatrix);

        [[nodiscard]]
        Matrix Transpose() const;

        void Set(float val);

        [[nodiscard]]
        const float &At(size_t iRow, size_t iCol) const;

        Matrix operator*(const Matrix &other) const;

        Matrix& operator*=(const Matrix &other);

        Matrix operator*(float f) const;

        Matrix operator*(const Vector3 &vec3) const;

        Matrix &operator*=(float f);

        Matrix &operator/=(float f);

        void Clear();

        [[nodiscard]]
        Matrix Strip(size_t row, size_t column) const;

        [[nodiscard]]
        float Minor(size_t i, size_t j) const;

        [[nodiscard]]
        float AlgComplement(size_t i, size_t j) const;

        [[nodiscard]]
        float Determinant() const;

        [[nodiscard]]
        const float* Raw() const;

        Matrix &operator=(const Matrix &other);

        Matrix &operator=(Matrix &&other) noexcept;

        Matrix operator/(float f) const;

        [[nodiscard]]
        std::string ToSrtring() const;

        ~Matrix();

    private:
        size_t m_rows = 0;
        size_t m_columns = 0;
        std::unique_ptr<float[]> m_data = nullptr;
    };
}