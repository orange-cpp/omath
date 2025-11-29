//
// Created by Vladislav on 09.11.2025.
//
#pragma once
#include "omath/linear_algebra/triangle.hpp"
#include <omath/linear_algebra/mat.hpp>
#include <omath/linear_algebra/vector3.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace omath::primitives
{
    template<class T = Vector3<float>>
    struct Vertex final
    {
        using VectorType = T;
        VectorType position;
        VectorType normal;
        VectorType uv;
    };

    template<typename T> concept HasPosition = requires(T vertex) { vertex.position; };
    template<typename T> concept HasNormal = requires(T vertex) { vertex.normal; };
    template<typename T> concept HasUv = requires(T vertex) { vertex.uv; };

    template<class Mat4X4, class RotationAngles, class MeshTypeTrait, class VertType = Vertex<>>
    class Mesh final
    {
    public:
        using VectorType = VertType::VectorType;
        using VertexType = VertType;

    private:
        using Vbo = std::vector<VertexType>;
        using Ebo = std::vector<Vector3<std::uint32_t>>;

    public:
        Vbo m_vertex_buffer;
        Ebo m_vertex_array_object;

        Mesh(Vbo vbo, Ebo vao,
             const VectorType scale =
                     {
                             1,
                             1,
                             1,
                     })
            : m_vertex_buffer(std::move(vbo)), m_vertex_array_object(std::move(vao)), m_scale(std::move(scale))
        {
        }
        void set_origin(const VectorType& new_origin)
        {
            m_origin = new_origin;
            m_to_world_matrix = std::nullopt;
        }

        void set_scale(const VectorType& new_scale)
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
        const VectorType& get_origin() const
        {
            return m_origin;
        }

        [[nodiscard]]
        const VectorType& get_scale() const
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
        VectorType vertex_to_world_space(const Vector3<float>& vertex_position) const
        requires HasPosition<VertexType>
        {
            auto abs_vec = get_to_world_matrix() * mat_column_from_vector(vertex_position);

            return {abs_vec.at(0, 0), abs_vec.at(1, 0), abs_vec.at(2, 0)};
        }

        [[nodiscard]]
        Triangle<VectorType> make_face_in_world_space(const Ebo::const_iterator vao_iterator) const
        requires HasPosition<VertexType>
        {
            return {vertex_to_world_space(m_vertex_buffer.at(vao_iterator->x).position),
                    vertex_to_world_space(m_vertex_buffer.at(vao_iterator->y).position),
                    vertex_to_world_space(m_vertex_buffer.at(vao_iterator->z).position)};
        }

    private:
        VectorType m_origin;
        VectorType m_scale;

        RotationAngles m_rotation_angles;

        mutable std::optional<Mat4X4> m_to_world_matrix;
    };
} // namespace omath::primitives