//
// Created by vlad on 9/29/2024.
//
#pragma once
#include <algorithm>
#include <array>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>
#include "omath/vector3.hpp"
#include <numeric>


#ifdef near
#undef near
#endif


#ifdef far
#undef far
#endif

namespace omath
{
    struct MatSize
    {
        size_t rows, columns;
    };

    enum class MatStoreType : uint8_t
    {
        ROW_MAJOR = 0,
        COLUMN_MAJOR
    };


    template<typename M1, typename M2>
    concept MatTemplateEqual =
            (M1::rows == M2::rows) && (M1::columns == M2::columns) &&
            std::is_same_v<typename M1::value_type, typename M2::value_type> &&
            (M1::store_type == M2::store_type);

    template<size_t Rows = 0, size_t Columns = 0, class Type = float, MatStoreType StoreType = MatStoreType::ROW_MAJOR>
        requires std::is_arithmetic_v<Type>
    class Mat final
    {
    public:
        constexpr Mat() noexcept
        {
            Clear();
        }

        [[nodiscard]]
        constexpr static MatStoreType GetStoreOrdering() noexcept
        {
            return StoreType;
        }
        constexpr Mat(const std::initializer_list<std::initializer_list<Type>>& rows)
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
                    At(i, j) = std::move(*colIt);
                }
            }
        }

        constexpr explicit Mat(const Type* rawData)
        {
            std::copy_n(rawData, Rows * Columns, m_data.begin());
        }

        constexpr Mat(const Mat& other) noexcept
        {
            m_data = other.m_data;
        }

        [[nodiscard]]
        constexpr Type& operator[](const size_t row, const size_t col)
        {
            return At(row, col);
        }

        [[nodiscard]]
        constexpr Type& operator[](const size_t row, const size_t col) const
        {
            return At(row, col);
        }

        constexpr Mat(Mat&& other) noexcept
        {
            m_data = std::move(other.m_data);
        }

        [[nodiscard]]
        static constexpr size_t RowCount() noexcept
        {
            return Rows;
        }

        [[nodiscard]]
        static constexpr size_t ColumnsCount() noexcept
        {
            return Columns;
        }

        [[nodiscard]]
        static consteval MatSize Size() noexcept
        {
            return {Rows, Columns};
        }

        [[nodiscard]]
        constexpr const Type& At(const size_t rowIndex, const size_t columnIndex) const
        {
#if !defined(NDEBUG) && defined(OMATH_SUPRESS_SAFETY_CHECKS)
            if (rowIndex >= Rows || columnIndex >= Columns)
                throw std::out_of_range("Index out of range");
#endif
            if constexpr (StoreType == MatStoreType::ROW_MAJOR)
                return m_data[rowIndex * Columns + columnIndex];

            else if constexpr (StoreType == MatStoreType::COLUMN_MAJOR)
                return m_data[rowIndex + columnIndex * Rows];

            else
            {
                static_assert(false, "Invalid matrix access convention");
                std::unreachable();
            }
        }

        [[nodiscard]] constexpr Type& At(const size_t rowIndex, const size_t columnIndex)
        {
            return const_cast<Type&>(std::as_const(*this).At(rowIndex, columnIndex));
        }

        [[nodiscard]]
        constexpr Type Sum() const noexcept
        {
            return std::accumulate(m_data.begin(), m_data.end(), Type(0));
        }

        constexpr void Clear() noexcept
        {
            Set(0);
        }

        constexpr void Set(const Type& value) noexcept
        {
            std::ranges::fill(m_data, value);
        }

        // Operator overloading for multiplication with another Mat
        template<size_t OtherColumns>
        [[nodiscard]]
        constexpr Mat<Rows, OtherColumns, Type, StoreType>
        operator*(const Mat<Columns, OtherColumns, Type, StoreType>& other) const
        {
            Mat<Rows, OtherColumns, Type, StoreType> result;

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

        constexpr Mat& operator*=(const Type& f) noexcept
        {
            std::ranges::for_each(m_data, [&f](auto& val) {val *= f;});
            return *this;
        }

        template<size_t OtherColumns>
        constexpr Mat<Rows, OtherColumns, Type, StoreType>
        operator*=(const Mat<Columns, OtherColumns, Type, StoreType>& other)
        {
            return *this = *this * other;
        }

        [[nodiscard]]
        constexpr Mat operator*(const Type& value) const noexcept
        {
            Mat result(*this);
            result *= value;
            return result;
        }

        constexpr Mat& operator/=(const Type& value) noexcept
        {
            std::ranges::for_each(m_data, [&value](auto& val) {val /= value;});
            return *this;
        }

        [[nodiscard]]
        constexpr Mat operator/(const Type& value) const noexcept
        {
            Mat result(*this);
            result /= value;
            return result;
        }

        constexpr Mat& operator=(const Mat& other) noexcept
        {
            if (this != &other)
                m_data = other.m_data;

            return *this;
        }

        constexpr Mat& operator=(Mat&& other) noexcept
        {
            if (this != &other)
                m_data = std::move(other.m_data);

            return *this;
        }

        [[nodiscard]]
        constexpr Mat<Columns, Rows, Type, StoreType> Transposed() const noexcept
        {
            Mat<Columns, Rows, Type, StoreType> transposed;
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

            if constexpr (Rows == 2)
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
        constexpr Mat<Rows - 1, Columns - 1, Type, StoreType> Minor(const size_t row, const size_t column) const
        {
            Mat<Rows - 1, Columns - 1, Type, StoreType> result;
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
        constexpr const std::array<Type, Rows * Columns>& RawArray() const
        {
            return m_data;
        }

        [[nodiscard]]
        constexpr std::array<Type, Rows * Columns>& RawArray()
        {
            return const_cast<std::array<Type, Rows * Columns>>(std::as_const(*this).RawArray());
        }

        [[nodiscard]]
        std::string ToString() const noexcept
        {
            std::ostringstream oss;
            oss << "[[";

            for (size_t i = 0; i < Rows; ++i)
            {
                if (i > 0)
                    oss << " [";

                for (size_t j = 0; j < Columns; ++j)
                {
                    oss << std::setw(9) << std::fixed << std::setprecision(3) << At(i, j);
                    if (j != Columns - 1)
                        oss << ", ";
                }
                oss << (i == Rows - 1 ? "]]" : "]\n");
            }
            return oss.str();
        }

        [[nodiscard]]
        bool operator==(const Mat& mat) const
        {
            return m_data == mat.m_data;
        }

        [[nodiscard]]
        bool operator!=(const Mat& mat) const
        {
            return !operator==(mat);
        }

        // Static methods that return fixed-size matrices
        [[nodiscard]]
        constexpr static Mat<4, 4> ToScreenMat(const Type& screenWidth, const Type& screenHeight) noexcept
        {
            return {
                    {screenWidth / 2, 0, 0, 0},
                    {0, -screenHeight / 2, 0, 0},
                    {0, 0, 1, 0},
                    {screenWidth / 2, screenHeight / 2, 0, 1},
            };
        }

    private:
        std::array<Type, Rows * Columns> m_data;
    };

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    [[nodiscard]]
    constexpr static Mat<1, 4, Type, St> MatRowFromVector(const Vector3<Type>& vector) noexcept
    {
        return {{vector.x, vector.y, vector.z, 1}};
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    [[nodiscard]]
    constexpr static Mat<4, 1, Type, St> MatColumnFromVector(const Vector3<Type>& vector) noexcept
    {
        return {{vector.x}, {vector.y}, {vector.z}, {1}};
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    [[nodiscard]]
    constexpr Mat<4, 4, Type, St> MatTranslation(const Vector3<Type>& diff) noexcept
    {
        return
        {
                {1, 0, 0, diff.x},
                {0, 1, 0, diff.y},
                {0, 0, 1, diff.z},
                {0, 0, 0, 1},
        };
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR, class Angle>
    [[nodiscard]]
    Mat<4, 4, Type, St> MatRotationAxisX(const Angle& angle) noexcept
    {
        return
        {
            {1, 0,           0,            0},
            {0, angle.Cos(), -angle.Sin(), 0},
            {0, angle.Sin(), angle.Cos(),  0},
            {0, 0,           0,            1}
        };
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR, class Angle>
    [[nodiscard]]
    Mat<4, 4, Type, St> MatRotationAxisY(const Angle& angle) noexcept
    {
        return
        {
            {angle.Cos(),  0, angle.Sin(), 0},
            {0           , 1,           0, 0},
            {-angle.Sin(), 0, angle.Cos(), 0},
            {0           , 0,           0, 1}
        };
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR, class Angle>
    [[nodiscard]]
    Mat<4, 4, Type, St> MatRotationAxisZ(const Angle& angle) noexcept
    {
        return
        {
            {angle.Cos(), -angle.Sin(), 0, 0},
            {angle.Sin(),  angle.Cos(), 0, 0},
            {          0,        0,     1, 0},
            {          0,        0,     0, 1},
        };
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    [[nodiscard]]
    static Mat<4, 4, Type, St> MatCameraView(const Vector3<Type>& forward, const Vector3<Type>& right,
                                             const Vector3<Type>& up, const Vector3<Type>& cameraOrigin) noexcept
    {
        return  Mat<4, 4, Type, St>
        {
            {right.x,   right.y,   right.z,   0},
            {up.x,      up.y,      up.z,      0},
            {forward.x, forward.y, forward.z, 0},
            {0,         0,         0,         1},

        } * MatTranslation<Type, St>(-cameraOrigin);
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR, class ViewAngles>
    [[nodiscard]]
    Mat<4, 4, Type, St> MatRotation(const ViewAngles& angles) noexcept
    {
        return MatRotationAxisZ(angles.yaw) * MatRotationAxisY(angles.pitch) * MatRotationAxisX(angles.roll);
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    [[nodiscard]]
    Mat<4, 4, Type, St> MatPerspectiveLeftHanded(const float fieldOfView, const float aspectRatio, const float near,
                                                 const float far) noexcept
    {
        const float fovHalfTan = std::tan(angles::DegreesToRadians(fieldOfView) / 2.f);

        return {{1.f / (aspectRatio * fovHalfTan), 0.f, 0.f, 0.f},
                {0.f, 1.f / fovHalfTan, 0.f, 0.f},
                {0.f, 0.f, (far + near) / (far - near), -(2.f * near * far) / (far - near)},
                {0.f, 0.f, 1.f, 0.f}};
    }

    template<class Type = float, MatStoreType St = MatStoreType::ROW_MAJOR>
    [[nodiscard]]
    Mat<4, 4, Type, St> MatPerspectiveRightHanded(const float fieldOfView, const float aspectRatio, const float near,
                                                  const float far) noexcept
    {
        const float fovHalfTan = std::tan(angles::DegreesToRadians(fieldOfView) / 2.f);

        return {{1.f / (aspectRatio * fovHalfTan), 0.f, 0.f, 0.f},
                {0.f, 1.f / fovHalfTan, 0.f, 0.f},
                {0.f, 0.f, -(far + near) / (far - near), -(2.f * near * far) / (far - near)},
                {0.f, 0.f, -1.f, 0.f}};
    }
} // namespace omath
