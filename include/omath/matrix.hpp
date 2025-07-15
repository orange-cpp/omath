#pragma once

#ifdef OMATH_ENABLE_LEGACY

#include "omath/vector3.hpp"
#include <initializer_list>
#include <memory>
#include <string>

namespace omath
{

    class Matrix final
    {
    public:
        Matrix();
        Matrix(size_t rows, size_t columns);

        Matrix(const std::initializer_list<std::initializer_list<float>>& rows);

        [[nodiscard]]
        static Matrix to_screen_matrix(float screen_width, float screen_height);

        [[nodiscard]]
        static Matrix translation_matrix(const Vector3<float>& diff);

        [[nodiscard]]
        static Matrix orientation_matrix(const Vector3<float>& forward, const Vector3<float>& right,
                                         const Vector3<float>& up);

        [[nodiscard]]
        static Matrix projection_matrix(float field_of_view, float aspect_ratio, float near, float far);

        Matrix(const Matrix& other);

        Matrix(size_t rows, size_t columns, const float* raw_data);

        Matrix(Matrix&& other) noexcept;

        [[nodiscard]]
        size_t row_count() const noexcept;

        [[nodiscard]]
        float& operator[](size_t row, size_t column);

        [[nodiscard]]
        size_t columns_count() const noexcept;

        [[nodiscard]]
        std::pair<size_t, size_t> size() const noexcept;

        [[nodiscard]]
        float& at(size_t row, size_t col);

        [[nodiscard]]
        float sum();

        void set_data_from_raw(const float* raw_matrix);

        [[nodiscard]]
        Matrix transpose() const;

        void set(float val);

        [[nodiscard]]
        const float& at(size_t row, size_t col) const;

        Matrix operator*(const Matrix& other) const;

        Matrix& operator*=(const Matrix& other);

        Matrix operator*(float f) const;

        Matrix& operator*=(float f);

        Matrix& operator/=(float f);

        void clear();

        [[nodiscard]]
        Matrix strip(size_t row, size_t column) const;

        [[nodiscard]]
        float minor(size_t i, size_t j) const;

        [[nodiscard]]
        float alg_complement(size_t i, size_t j) const;

        [[nodiscard]]
        float determinant() const;

        [[nodiscard]]
        const float* raw() const;

        Matrix& operator=(const Matrix& other);

        Matrix& operator=(Matrix&& other) noexcept;

        Matrix operator/(float f) const;

        [[nodiscard]]
        std::string to_string() const;

        ~Matrix();

    private:
        size_t m_rows;
        size_t m_columns;
        std::unique_ptr<float[]> m_data;
    };
} // namespace omath
#endif
