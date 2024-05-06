#pragma once
#include <vector>
#include <memory>

namespace uml
{
    class Vector3;

    class matrix
    {
    public:
        matrix(size_t rows, size_t columns);

        explicit matrix(const std::vector<std::vector<float>> &rows);

        [[nodiscard]] static matrix to_screen_matrix(float screenWidth, float screenHeight);

        matrix(const matrix &other);

        matrix(size_t rows, size_t columns, const float *pRaw);

        matrix(matrix &&other) noexcept;

        [[nodiscard]] size_t get_rows_count() const noexcept;

        [[nodiscard]] size_t get_columns_count() const noexcept;

        [[nodiscard]] std::pair<size_t, size_t> get_size() const noexcept;

        float &at(size_t iRow, size_t iCol);

        float get_sum();
        void set_from_raw(const float* pRawMatrix);
        matrix transpose();

        void set(float val);

        [[nodiscard]] const float &at(size_t iRow, size_t iCol) const;

        matrix operator*(const matrix &other) const;

        matrix operator*(float f) const;

        matrix operator*(const Vector3 &vec3) const;

        matrix &operator*=(float f);

        matrix &operator/=(float f);

        void clear();

        [[nodiscard]] matrix strip(size_t row, size_t column) const;

        [[nodiscard]] float minor(size_t i, size_t j) const;

        [[nodiscard]] float alg_complement(size_t i, size_t j) const;

        [[nodiscard]] float det() const;
        [[nodiscard]] const float* raw() const;
        matrix &operator=(const matrix &other);

        matrix &operator=(matrix &&other) noexcept;

        matrix operator/(float f) const;

        ~matrix();

    private:
        size_t m_rows = 0;
        size_t m_columns = 0;
        std::unique_ptr<float[]> m_pData = nullptr;
    };
}