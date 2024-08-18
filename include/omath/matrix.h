#pragma once
#include <vector>
#include <memory>
#include <string>


namespace omath
{
    class Vector3;

    class Matrix
    {
    public:
        Matrix(size_t rows, size_t columns);

        explicit Matrix(const std::vector<std::vector<float>> &rows);

        [[nodiscard]]
        static Matrix to_screen_matrix(float screenWidth, float screenHeight);

        Matrix(const Matrix &other);

        Matrix(size_t rows, size_t columns, const float *pRaw);

        Matrix(Matrix &&other) noexcept;

        [[nodiscard]]
        size_t get_rows_count() const noexcept;

        [[nodiscard]]
        size_t get_columns_count() const noexcept;

        [[nodiscard]]
        std::pair<size_t, size_t> get_size() const noexcept;

        [[nodiscard]]
        float &at(size_t iRow, size_t iCol);

        [[nodiscard]]
        float get_sum();

        void set_from_raw(const float* pRawMatrix);

        [[nodiscard]]
        Matrix transpose();

        void set(float val);

        [[nodiscard]]
        const float &at(size_t iRow, size_t iCol) const;

        Matrix operator*(const Matrix &other) const;

        Matrix operator*(float f) const;

        Matrix operator*(const Vector3 &vec3) const;

        Matrix &operator*=(float f);

        Matrix &operator/=(float f);

        void clear();

        [[nodiscard]]
        Matrix strip(size_t row, size_t column) const;

        [[nodiscard]]
        float minor(size_t i, size_t j) const;

        [[nodiscard]]
        float alg_complement(size_t i, size_t j) const;

        [[nodiscard]]
        float det() const;

        [[nodiscard]]
        const float* raw() const;

        Matrix &operator=(const Matrix &other);

        Matrix &operator=(Matrix &&other) noexcept;

        Matrix operator/(float f) const;

        [[nodiscard]]
        std::string to_string() const;

        ~Matrix();

    private:
        size_t m_rows = 0;
        size_t m_columns = 0;
        std::unique_ptr<float[]> m_pData = nullptr;
    };
}