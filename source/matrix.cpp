#include "omath/Matrix.h"

#include <format>
#include "omath/Vector3.h"
#include <utility>
#include <stdexcept>
#include <utility>
#include <cmath>


namespace omath
{
    Matrix::Matrix(const size_t rows, const size_t columns)
    {
        if (rows == 0 and columns == 0)
            throw std::runtime_error("Matrix cannot be 0x0");

        m_rows = rows;
        m_columns = columns;

        m_data = std::make_unique<float[]>(m_rows * m_columns);

        Set(0.f);
    }

    Matrix::Matrix(const std::vector<std::vector<float>> &rows)
    {
        m_rows = rows.size();
        m_columns = rows[0].size();


        m_data = std::make_unique<float[]>(m_rows * m_columns);

        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                At(i,j) = rows[i][j];
    }

    Matrix::Matrix(const Matrix &other)
    {
        m_rows = other.m_rows;
        m_columns = other.m_columns;

        m_data = std::make_unique<float[]>(m_rows * m_columns);

        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                At(i, j) = other.At(i, j);
    }

    Matrix::Matrix(const size_t rows, const size_t columns, const float *pRaw)
    {
        m_rows = rows;
        m_columns = columns;


        m_data = std::make_unique<float[]>(m_rows * m_columns);

        for (size_t i = 0; i < rows*columns; ++i)
            At(i / rows, i % columns) = pRaw[i];

    }

    size_t Matrix::RowCount() const noexcept
    {
        return m_rows;
    }

    Matrix::Matrix(Matrix &&other) noexcept
    {
        m_rows = other.m_rows;
        m_columns = other.m_columns;
        m_data = std::move(other.m_data);

    }

    size_t Matrix::ColumnsCount() const noexcept
    {
        return m_columns;
    }

    std::pair<size_t, size_t> Matrix::Size() const noexcept
    {
        return {RowCount(), ColumnsCount()};
    }

    float &Matrix::At(const size_t iRow, const size_t iCol)
    {
        return const_cast<float&>(std::as_const(*this).At(iRow, iCol));
    }

    float Matrix::Sum()
    {
        float sum = 0;

        for (size_t i = 0; i < RowCount(); i++)
            for (size_t j = 0; j < ColumnsCount(); j++)
                sum += At(i, j);

        return sum;
    }

    const float &Matrix::At(const size_t iRow, const size_t iCol) const
    {
        return m_data[iRow * m_columns + iCol];
    }

    Matrix Matrix::operator*(const Matrix &other) const
    {
        if (m_columns != other.m_rows)
            throw std::runtime_error("n != m");

        auto outMat = Matrix(m_rows, other.m_columns);

        for (size_t d = 0; d < m_rows; ++d)
            for (size_t i = 0; i < other.m_columns; ++i)
                for (size_t j = 0; j < other.m_rows; ++j)
                    outMat.At(d, i) += At(d, j) * other.At(j, i);


        return outMat;
    }

    Matrix & Matrix::operator*=(const Matrix &other)
    {
        *this = *this * other;
        return *this;
    }

    Matrix Matrix::operator*(const float f) const
    {
        auto out = *this;
        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                out.At(i, j) *= f;

        return out;
    }

    Matrix &Matrix::operator*=(const float f)
    {
        for (size_t i = 0; i < RowCount(); i++)
            for (size_t j = 0; j < ColumnsCount(); j++)
                At(i, j) *= f;
        return *this;
    }

    void Matrix::Clear()
    {
        Set(0.f);
    }

    Matrix Matrix::operator*(const Vector3 &vec3) const
    {
        auto vecmatrix = Matrix(m_rows, 1);
        vecmatrix.Set(1.f);
        vecmatrix.At(0, 0) = vec3.x;
        vecmatrix.At(1, 0) = vec3.y;
        vecmatrix.At(2, 0) = vec3.z;

        return *this * vecmatrix;

    }


    Matrix &Matrix::operator=(const Matrix &other)
    {
        if (this == &other)
            return *this;

        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                At(i, j) = other.At(i, j);

        return *this;

    }

    Matrix &Matrix::operator=(Matrix &&other) noexcept
    {
        if (this == &other)
            return *this;

        m_rows = other.m_rows;
        m_columns = other.m_columns;
        m_data = std::move(other.m_data);

        return *this;

    }

    Matrix &Matrix::operator/=(const float f)
    {
        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                At(i, j) /= f;

        return *this;
    }

    Matrix Matrix::operator/(const float f) const
    {
        auto out = *this;
        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                out.At(i, j) /= f;

        return out;
    }

    std::string Matrix::ToSrtring() const
    {
        std::string str;

        for (size_t i = 0; i < m_rows; i++)
        {
            for (size_t j = 0; j < m_columns; ++j)
            {
                str += std::format("{:.1f}",At(i, j));

                if (j == m_columns-1)
                    str += '\n';
                else
                    str += ' ';
            }
        }
        return str;
    }

    float Matrix::Determinant() const
    {
        if (m_rows + m_columns == 2)
            return At(0, 0);

        if (m_rows == 2 and m_columns == 2)
            return At(0, 0) * At(1, 1) - At(0, 1) * At(1, 0);

        float fDet = 0;
        for (size_t i = 0; i < m_columns; i++)
            fDet += AlgComplement(0, i) * At(0, i);

        return fDet;
    }

    float Matrix::AlgComplement(const size_t i, const size_t j) const
    {
        const auto tmp = Minor(i, j);
        return ((i + j + 2) % 2 == 0) ? tmp : -tmp;
    }

    Matrix Matrix::Transpose()
    {
        Matrix transposed = {m_columns, m_rows};

        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                transposed.At(j, i) = At(i, j);

        return transposed;
    }

    Matrix::~Matrix() = default;

    void Matrix::Set(const float val)
    {
        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_columns; ++j)
                At(i, j) = val;
    }

    Matrix Matrix::Strip(const size_t row, const size_t column) const
    {
        Matrix stripped = {m_rows - 1, m_columns - 1};
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

                stripped.At(iStripRowIndex, iStripColumnIndex) = At(i, j);
                iStripColumnIndex++;
            }

            iStripRowIndex++;
        }

        return stripped;
    }

    float Matrix::Minor(const size_t i, const size_t j) const
    {
        return Strip(i, j).Determinant();
    }

    Matrix Matrix::ToScreenMatrix(float screenWidth, float screenHeight)
    {
        return Matrix({
        {screenWidth / 2.f, 0.f,                 0.f, 0.f},
        {0.f,               -screenHeight / 2.f, 0.f, 0.f},
        {0.f,               0.f,                 1.f, 0.f},
        {screenWidth / 2.f, screenHeight / 2.f,  0.f, 1.f},
       });
    }

    const float * Matrix::Raw() const
    {
        return m_data.get();
    }

    void Matrix::SetDataFromRaw(const float *pRawMatrix)
    {
        for (size_t i = 0; i < m_columns*m_rows; ++i)
            At(i / m_rows, i % m_columns) = pRawMatrix[i];
    }
}