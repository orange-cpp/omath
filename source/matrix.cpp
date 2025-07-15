#ifdef OMATH_ENABLE_LEGACY

#include "omath/matrix.hpp"
#include "omath/angles.hpp"
#include "omath/vector3.hpp"
#include <complex>
#include <format>
#include <stdexcept>
#include <utility>

namespace omath
{
    Matrix::Matrix(const size_t rows, const size_t columns)
    {
        if (rows == 0 and columns == 0)
            throw std::runtime_error("Matrix cannot be 0x0");

        m_rows = rows;
        m_columns = columns;

        m_data = std::make_unique<float[]>(m_rows * m_columns);

        set(0.f);
    }

    Matrix::Matrix(const std::initializer_list<std::initializer_list<float>>& rows)
    {
        m_rows = rows.size();
        m_columns = rows.begin()->size();

        for (const auto& row: rows)
            if (row.size() != m_columns)
                throw std::invalid_argument("All rows must have the same number of columns.");

        m_data = std::make_unique<float[]>(m_rows * m_columns);

        size_t i = 0;
        for (const auto& row: rows)
        {
            size_t j = 0;
            for (const auto& value: row)
                at(i, j++) = value;
            ++i;
        }
    }

    Matrix::Matrix(const Matrix& other)
    {
        m_rows = other.m_rows;
        m_columns = other.m_columns;

        m_data = std::make_unique<float[]>(m_rows * m_columns);

        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                at(i, j) = other.at(i, j);
    }

    Matrix::Matrix(const size_t rows, const size_t columns, const float* raw_data)
    {
        m_rows = rows;
        m_columns = columns;

        m_data = std::make_unique<float[]>(m_rows * m_columns);

        for (size_t i = 0; i < rows * columns; ++i)
            at(i / rows, i % columns) = raw_data[i];
    }

    size_t Matrix::row_count() const noexcept
    {
        return m_rows;
    }

    float& Matrix::operator[](const size_t row, const size_t column)
    {
        return at(row, column);
    }

    Matrix::Matrix(Matrix&& other) noexcept
    {
        m_rows = other.m_rows;
        m_columns = other.m_columns;
        m_data = std::move(other.m_data);

        other.m_rows = 0;
        other.m_columns = 0;

        other.m_data = nullptr;
    }

    size_t Matrix::columns_count() const noexcept
    {
        return m_columns;
    }

    std::pair<size_t, size_t> Matrix::size() const noexcept
    {
        return {row_count(), columns_count()};
    }

    float& Matrix::at(const size_t row, const size_t col)
    {
        return const_cast<float&>(std::as_const(*this).at(row, col));
    }

    float Matrix::sum()
    {
        float sum = 0;

        for (size_t i = 0; i < row_count(); i++)
            for (size_t j = 0; j < columns_count(); j++)
                sum += at(i, j);

        return sum;
    }

    const float& Matrix::at(const size_t row, const size_t col) const
    {
        return m_data[row * m_columns + col];
    }

    Matrix Matrix::operator*(const Matrix& other) const
    {
        if (m_columns != other.m_rows)
            throw std::runtime_error("n != m");

        auto out_mat = Matrix(m_rows, other.m_columns);

        for (size_t d = 0; d < m_rows; ++d)
            for (size_t i = 0; i < other.m_columns; ++i)
                for (size_t j = 0; j < other.m_rows; ++j)
                    out_mat.at(d, i) += at(d, j) * other.at(j, i);

        return out_mat;
    }

    Matrix& Matrix::operator*=(const Matrix& other)
    {
        *this = *this * other;
        return *this;
    }

    Matrix Matrix::operator*(const float f) const
    {
        auto out = *this;
        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                out.at(i, j) *= f;

        return out;
    }

    Matrix& Matrix::operator*=(const float f)
    {
        for (size_t i = 0; i < row_count(); i++)
            for (size_t j = 0; j < columns_count(); j++)
                at(i, j) *= f;
        return *this;
    }

    void Matrix::clear()
    {
        set(0.f);
    }

    Matrix& Matrix::operator=(const Matrix& other)
    {
        if (this == &other)
            return *this;

        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                at(i, j) = other.at(i, j);

        return *this;
    }

    Matrix& Matrix::operator=(Matrix&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_rows = other.m_rows;
        m_columns = other.m_columns;
        m_data = std::move(other.m_data);

        other.m_rows = 0;
        other.m_columns = 0;

        return *this;
    }

    Matrix& Matrix::operator/=(const float f)
    {
        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                at(i, j) /= f;

        return *this;
    }

    Matrix Matrix::operator/(const float f) const
    {
        auto out = *this;
        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                out.at(i, j) /= f;

        return out;
    }

    std::string Matrix::to_string() const
    {
        std::string str;

        for (size_t i = 0; i < m_rows; i++)
        {
            for (size_t j = 0; j < m_columns; ++j)
            {
                str += std::format("{:.1f}", at(i, j));

                if (j == m_columns - 1)
                    str += '\n';
                else
                    str += ' ';
            }
        }
        return str;
    }

    float Matrix::determinant() const // NOLINT(*-no-recursion)
    {
        if (m_rows + m_columns == 2)
            return at(0, 0);

        if (m_rows == 2 and m_columns == 2)
            return at(0, 0) * at(1, 1) - at(0, 1) * at(1, 0);

        float det = 0;
        for (size_t i = 0; i < m_columns; i++)
            det += alg_complement(0, i) * at(0, i);

        return det;
    }

    float Matrix::alg_complement(const size_t i, const size_t j) const // NOLINT(*-no-recursion)
    {
        const auto tmp = minor(i, j);
        return ((i + j + 2) % 2 == 0) ? tmp : -tmp;
    }

    Matrix Matrix::transpose() const
    {
        Matrix transposed = {m_columns, m_rows};

        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                transposed.at(j, i) = at(i, j);

        return transposed;
    }

    Matrix::~Matrix() = default;

    void Matrix::set(const float val)
    {
        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                at(i, j) = val;
    }

    Matrix Matrix::strip(const size_t row, const size_t column) const
    {
        Matrix stripped = {m_rows - 1, m_columns - 1};
        size_t strip_row_index = 0;

        for (size_t i = 0; i < m_rows; i++)
        {
            if (i == row)
                continue;

            size_t strip_column_index = 0;
            for (size_t j = 0; j < m_columns; ++j)
            {
                if (j == column)
                    continue;

                stripped.at(strip_row_index, strip_column_index) = at(i, j);
                strip_column_index++;
            }

            strip_row_index++;
        }

        return stripped;
    }

    float Matrix::minor(const size_t i, const size_t j) const // NOLINT(*-no-recursion)
    {
        return strip(i, j).determinant();
    }

    Matrix Matrix::to_screen_matrix(const float screen_width, const float screen_height)
    {
        return {
                {screen_width / 2.f, 0.f, 0.f, 0.f},
                {0.f, -screen_height / 2.f, 0.f, 0.f},
                {0.f, 0.f, 1.f, 0.f},
                {screen_width / 2.f, screen_height / 2.f, 0.f, 1.f},
        };
    }

    Matrix Matrix::translation_matrix(const Vector3<float>& diff)
    {
        return {
                {1.f, 0.f, 0.f, 0.f},
                {0.f, 1.f, 0.f, 0.f},
                {0.f, 0.f, 1.f, 0.f},
                {diff.x, diff.y, diff.z, 1.f},
        };
    }

    Matrix Matrix::orientation_matrix(const Vector3<float>& forward, const Vector3<float>& right,
                                      const Vector3<float>& up)
    {
        return {
                {right.x, up.x, forward.x, 0.f},
                {right.y, up.y, forward.y, 0.f},
                {right.z, up.z, forward.z, 0.f},
                {0.f, 0.f, 0.f, 1.f},
        };
    }

    Matrix Matrix::projection_matrix(const float field_of_view, const float aspect_ratio, const float near,
                                     const float far)
    {
        const float fov_half_tan = std::tan(angles::degrees_to_radians(field_of_view) / 2.f);

        return {{1.f / (aspect_ratio * fov_half_tan), 0.f, 0.f, 0.f},
                {0.f, 1.f / fov_half_tan, 0.f, 0.f},
                {0.f, 0.f, (far + near) / (far - near), 2.f * near * far / (far - near)},
                {0.f, 0.f, -1.f, 0.f}};
    }

    const float* Matrix::raw() const
    {
        return m_data.get();
    }

    void Matrix::set_data_from_raw(const float* raw_matrix)
    {
        for (size_t i = 0; i < m_columns * m_rows; ++i)
            at(i / m_rows, i % m_columns) = raw_matrix[i];
    }

    Matrix::Matrix()
    {
        m_columns = 0;
        m_rows = 0;
        m_data = nullptr;
    }
} // namespace omath
#endif
