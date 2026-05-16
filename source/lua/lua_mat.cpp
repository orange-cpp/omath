//
// Created by orange on 07.03.2026.
//
#ifdef OMATH_ENABLE_LUA
#include "omath/lua/lua.hpp"
#include <omath/linear_algebra/mat.hpp>
#include <sol/sol.hpp>
#include <stdexcept>

namespace
{
    template<std::size_t Limit>
    std::size_t checked_index(const int index)
    {
        if (index < 0 || index >= static_cast<int>(Limit))
            throw std::out_of_range("matrix index is out of range");
        return static_cast<std::size_t>(index);
    }

    template<std::size_t Rows, std::size_t Columns, class Type, omath::MatStoreType StoreType>
    omath::Mat<Rows, Columns, Type, StoreType> mat_from_rows(const sol::table& rows)
    {
        omath::Mat<Rows, Columns, Type, StoreType> result;

        for (std::size_t row = 0; row < Rows; ++row)
        {
            const auto row_table = rows[row + 1].get<sol::optional<sol::table>>();
            if (!row_table)
                throw std::invalid_argument("matrix rows must be tables");

            for (std::size_t column = 0; column < Columns; ++column)
            {
                const auto value = (*row_table)[column + 1].get<sol::optional<Type>>();
                if (!value)
                    throw std::invalid_argument("matrix row has missing value");
                result.at(row, column) = *value;
            }
        }

        return result;
    }

    template<std::size_t Rows, std::size_t Columns, class Type, omath::MatStoreType StoreType>
    sol::table mat_as_table(const omath::Mat<Rows, Columns, Type, StoreType>& mat, sol::this_state state)
    {
        sol::state_view lua(state);
        sol::table rows = lua.create_table();

        for (std::size_t row = 0; row < Rows; ++row)
        {
            sol::table row_table = lua.create_table();
            for (std::size_t column = 0; column < Columns; ++column)
                row_table[column + 1] = mat.at(row, column);
            rows[row + 1] = row_table;
        }

        return rows;
    }

    template<std::size_t Size, class Type, omath::MatStoreType StoreType, bool RegisterMat4Helpers = false>
    void register_square_mat(sol::table& omath_table, const char* name)
    {
        using MatType = omath::Mat<Size, Size, Type, StoreType>;

        auto type = omath_table.new_usertype<MatType>(
                name, sol::constructors<MatType()>(),

                "from_rows", &mat_from_rows<Size, Size, Type, StoreType>, "row_count",
                []()
                {
                    return Size;
                },
                "columns_count",
                []()
                {
                    return Size;
                },

                "at",
                [](const MatType& mat, const int row, const int column)
                {
                    return mat.at(checked_index<Size>(row), checked_index<Size>(column));
                },
                "set_at",
                [](MatType& mat, const int row, const int column, const Type value)
                {
                    mat.at(checked_index<Size>(row), checked_index<Size>(column)) = value;
                    return mat;
                },

                sol::meta_function::multiplication,
                sol::overload(
                        [](const MatType& lhs, const MatType& rhs)
                        {
                            return lhs * rhs;
                        },
                        [](const MatType& mat, const Type scalar)
                        {
                            return mat * scalar;
                        },
                        [](const Type scalar, const MatType& mat)
                        {
                            return mat * scalar;
                        }),
                sol::meta_function::division,
                [](const MatType& mat, const Type scalar)
                {
                    return mat / scalar;
                },
                sol::meta_function::equal_to, &MatType::operator==, sol::meta_function::to_string, &MatType::to_string,

                "transposed", &MatType::transposed, "determinant", &MatType::determinant, "sum", &MatType::sum, "clear",
                &MatType::clear, "set", &MatType::set, "inverted",
                [](const MatType& mat) -> sol::optional<MatType>
                {
                    auto result = mat.inverted();
                    if (!result)
                        return sol::nullopt;
                    return *result;
                },
                "as_table", &mat_as_table<Size, Size, Type, StoreType>);

        if constexpr (RegisterMat4Helpers)
        {
            type["to_screen_mat"] = [](const Type screen_width, const Type screen_height)
            {
                return MatType{
                        {screen_width / Type{2}, Type{0}, Type{0}, Type{0}},
                        {Type{0}, -screen_height / Type{2}, Type{0}, Type{0}},
                        {Type{0}, Type{0}, Type{1}, Type{0}},
                        {screen_width / Type{2}, screen_height / Type{2}, Type{0}, Type{1}},
                };
            };
            type["translation"] = &omath::mat_translation<Type, StoreType>;
            type["scale"] = &omath::mat_scale<Type, StoreType>;
            type["look_at_left_handed"] = &omath::mat_look_at_left_handed<Type, StoreType>;
            type["look_at_right_handed"] = &omath::mat_look_at_right_handed<Type, StoreType>;
            type["perspective_left_handed_vertical_fov"] =
                    &omath::mat_perspective_left_handed_vertical_fov<Type, StoreType>;
            type["perspective_right_handed_vertical_fov"] =
                    &omath::mat_perspective_right_handed_vertical_fov<Type, StoreType>;
        }
    }
} // namespace

namespace omath::lua
{
    void LuaInterpreter::register_matrices(sol::table& omath_table)
    {
        register_square_mat<3, float, omath::MatStoreType::ROW_MAJOR>(omath_table, "Mat3");
        register_square_mat<4, float, omath::MatStoreType::ROW_MAJOR, true>(omath_table, "Mat4");
        register_square_mat<4, float, omath::MatStoreType::COLUMN_MAJOR, true>(omath_table, "Mat4ColumnMajor");
        register_square_mat<4, double, omath::MatStoreType::ROW_MAJOR>(omath_table, "Mat4d");
    }
} // namespace omath::lua
#endif
