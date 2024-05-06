/*
 * Copyright (c) 2022.
 * Created by Alpatov Softworks with love in Russia.
 */

#include "uml/matrix.h"
#include "uml/Vector3.h"
#include <utility>
#include <stdexcept>
#include <utility>


namespace uml
{
    matrix::matrix(const size_t rows, const size_t columns)
    {
        if (rows == 0 and columns == 0)
            throw std::runtime_error("Matrix cannot be 0x0");

        m_rows = rows;
        m_columns = columns;

        m_pData = std::make_unique<float[]>(m_rows * m_columns);

        set(0.f);
    }

    matrix::matrix(const std::vector<std::vector<float>> &rows)
    {
        m_rows = rows.size();
        m_columns = rows[0].size();


        m_pData = std::make_unique<float[]>(m_rows * m_columns);

        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                at(i,j) = rows[i][j];
    }

    matrix::matrix(const matrix &other)
    {
        m_rows = other.m_rows;
        m_columns = other.m_columns;

        m_pData = std::make_unique<float[]>(m_rows * m_columns);

        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                at(i, j) = other.at(i, j);
    }

    matrix::matrix(const size_t rows, const size_t columns, const float *pRaw)
    {
        m_rows = rows;
        m_columns = columns;


        m_pData = std::make_unique<float[]>(m_rows * m_columns);

        for (size_t i = 0; i < rows*columns; ++i)
            at(i / rows, i % columns) = pRaw[i];

    }

    size_t matrix::get_rows_count() const noexcept
    {
        return m_rows;
    }

    matrix::matrix(matrix &&other) noexcept
    {
        m_rows = other.m_rows;
        m_columns = other.m_columns;
        m_pData = std::move(other.m_pData);

    }

    size_t matrix::get_columns_count() const noexcept
    {
        return m_columns;
    }

    std::pair<size_t, size_t> matrix::get_size() const noexcept
    {
        return {get_rows_count(), get_columns_count()};
    }

    float &matrix::at(const size_t iRow, const size_t iCol)
    {
        return const_cast<float&>(std::as_const(*this).at(iRow, iCol));
    }

    float matrix::get_sum()
    {
        float sum = 0;

        for (size_t i = 0; i < get_rows_count(); i++)
            for (size_t j = 0; j < get_columns_count(); j++)
                sum += at(i, j);

        return sum;
    }

    const float &matrix::at(const size_t iRow, const size_t iCol) const
    {
        return m_pData[iRow * m_columns + iCol];
    }

    matrix matrix::operator*(const matrix &other) const
    {
        if (m_columns != other.m_rows)
            throw std::runtime_error("n != m");

        auto outMat = matrix(m_rows, other.m_columns);

        for (size_t d = 0; d < m_rows; ++d)
            for (size_t i = 0; i < other.m_columns; ++i)
                for (size_t j = 0; j < other.m_rows; ++j)
                    outMat.at(d, i) += at(d, j) * other.at(j, i);


        return outMat;
    }

    matrix matrix::operator*(const float f) const
    {
        auto out = *this;
        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                out.at(i, j) *= f;

        return out;
    }

    matrix &matrix::operator*=(const float f)
    {
        for (size_t i = 0; i < get_rows_count(); i++)
            for (size_t j = 0; j < get_columns_count(); j++)
                at(i, j) *= f;
        return *this;
    }

    void matrix::clear()
    {
        set(0.f);
    }

    matrix matrix::operator*(const Vector3 &vec3) const
    {
        auto vecmatrix = matrix(m_rows, 1);
        vecmatrix.set(1.f);
        vecmatrix.at(0, 0) = vec3.x;
        vecmatrix.at(1, 0) = vec3.y;
        vecmatrix.at(2, 0) = vec3.z;

        return *this * vecmatrix;

    }


    matrix &matrix::operator=(const matrix &other)
    {
        if (this == &other)
            return *this;

        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                at(i, j) = other.at(i, j);

        return *this;

    }

    matrix &matrix::operator=(matrix &&other) noexcept
    {
        if (this == &other)
            return *this;

        m_rows = other.m_rows;
        m_columns = other.m_columns;
        m_pData = std::move(other.m_pData);

        return *this;

    }

    matrix &matrix::operator/=(const float f)
    {
        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                at(i, j) /= f;

        return *this;
    }

    matrix matrix::operator/(const float f) const
    {
        auto out = *this;
        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                out.at(i, j) /= f;

        return out;
    }

    float matrix::det() const
    {
        if (m_rows + m_columns == 2)
            return at(0, 0);

        if (m_rows == 2 and m_columns == 2)
            return at(0, 0) * at(1, 1) - at(0, 1) * at(1, 0);

        float fDet = 0;
        for (size_t i = 0; i < m_columns; i++)
            fDet += alg_complement(0, i) * at(0, i);

        return fDet;
    }

    float matrix::alg_complement(const size_t i, const size_t j) const
    {
        const auto tmp = minor(i, j);
        return ((i + j + 2) % 2 == 0) ? tmp : -tmp;
    }

    matrix matrix::transpose()
    {
        matrix transposed = {m_columns, m_rows};

        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                transposed.at(j, i) = at(i, j);

        return transposed;
    }

    matrix::~matrix() = default;

    void matrix::set(const float val)
    {
        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                at(i, j) = val;
    }

    matrix matrix::strip(const size_t row, const size_t column) const
    {
        matrix stripped = {m_rows - 1, m_columns - 1};
        size_t iStripRowIndex = 0;

        for (size_t i = 0; i < m_rows; i++)
        {
            if (i == row)
                continue;

            size_t iStripColumnIndex = 0;
            for (size_t j = 0; j < m_columns; ++j)
            {
                if (j == column)
                    continue;

                stripped.at(iStripRowIndex, iStripColumnIndex) = at(i, j);
                iStripColumnIndex++;
            }

            iStripRowIndex++;
        }

        return stripped;
    }

    float matrix::minor(const size_t i, const size_t j) const
    {
        return strip(i, j).det();
    }

    matrix matrix::to_screen_matrix(float screenWidth, float screenHeight)
    {
        return matrix({
                              {screenWidth / 2.f, 0.f,                 0.f, 0.f},
                              {0.f,               -screenHeight / 2.f, 0.f, 0.f},
                              {0.f,               0.f,                 1.f, 0.f},
                              {screenWidth / 2.f, screenHeight / 2.f,  0.f, 1.f},
                      });
    }

    const float * matrix::raw() const
    {
        return m_pData.get();
    }

    void matrix::set_from_raw(const float *pRawMatrix)
    {
        for (size_t i = 0; i < m_columns*m_rows; ++i)
            at(i / m_rows, i % m_columns) = pRawMatrix[i];
    }
}