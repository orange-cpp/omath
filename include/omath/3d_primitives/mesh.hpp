//
// Created by Vladislav on 09.11.2025.
//
#pragma once
#include "omath/linear_algebra/triangle.hpp"
#include <omath/linear_algebra/mat.hpp>
#include <omath/linear_algebra/vector3.hpp>
#include <utility>
#include <vector>

namespace omath::primitives
{
    template<class Mat4X4, class RotationAngles, class MeshTypeTrait, class Type = float>
    class Mesh final
    {
    public:
        using NumericType = Type;

    private:
        using Vbo = std::vector<Vector3<NumericType>>;
        using Vao = std::vector<Vector3<std::size_t>>;

    public:
        Vbo m_vertex_buffer;
        Vao m_vertex_array_object;

        Mesh(Vbo vbo, Vao vao, const Vector3<NumericType> scale = {1, 1, 1,})
            : m_vertex_buffer(std::move(vbo)), m_vertex_array_object(std::move(vao)), m_scale(scale)
        {
        }
        void set_origin(const Vector3<NumericType>& new_origin)
        {
            m_origin = new_origin;
            m_to_world_matrix = std::nullopt;
        }

        void set_scale(const Vector3<NumericType>& new_scale)
        {
            m_scale = new_scale;
            m_to_world_matrix = std::nullopt;
        }

        void set_rotation(const RotationAngles& new_rotation_angles)
        {
            m_rotation_angles = new_rotation_angles;
            m_to_world_matrix = std::nullopt;
        }

        [[nodiscard]]
        const Vector3<NumericType>& get_origin() const
        {
            return m_origin;
        }

        [[nodiscard]]
        const Vector3<NumericType>& get_scale() const
        {
            return m_scale;
        }

        [[nodiscard]]
        const RotationAngles& get_rotation_angles() const
        {
            return m_rotation_angles;
        }

        [[nodiscard]]
        const Mat4X4& get_to_world_matrix() const
        {
            if (m_to_world_matrix)
                return m_to_world_matrix.value();
            m_to_world_matrix =
                    mat_translation(m_origin) * mat_scale(m_scale) * MeshTypeTrait::rotation_matrix(m_rotation_angles);

            return m_to_world_matrix.value();
        }

        [[nodiscard]]
        Vector3<float> vertex_to_world_space(const Vector3<float>& vertex) const
        {
            auto abs_vec = get_to_world_matrix() * mat_column_from_vector(vertex);

            return {abs_vec.at(0, 0), abs_vec.at(1, 0), abs_vec.at(2, 0)};
        }

        [[nodiscard]]
        Triangle<Vector3<float>> make_face_in_world_space(const Vao::const_iterator vao_iterator) const
        {
            return {vertex_to_world_space(m_vertex_buffer.at(vao_iterator->x)),
                    vertex_to_world_space(m_vertex_buffer.at(vao_iterator->y)),
                    vertex_to_world_space(m_vertex_buffer.at(vao_iterator->z))};
        }

    private:
        Vector3<NumericType> m_origin;
        Vector3<NumericType> m_scale;

        RotationAngles m_rotation_angles;

        mutable std::optional<Mat4X4> m_to_world_matrix;
    };
} // namespace omath::primitives