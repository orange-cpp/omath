//
// Created by Vladislav on 09.11.2025.
//
#pragma once
#include "aabb.hpp"
#include "omath/linear_algebra/triangle.hpp"
#include <limits>
#include <omath/linear_algebra/mat.hpp>
#include <omath/linear_algebra/vector3.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace omath::primitives
{
    template<class VecType = Vector3<float>, class UvT = Vector2<float>>
    struct Vertex final
    {
        using VectorType = VecType;
        using UvType = UvT;
        VectorType position;
        VectorType normal;
        UvType uv;
    };

    template<typename T> concept HasPosition = requires(T vertex) { vertex.position; };
    template<typename T> concept HasNormal = requires(T vertex) { vertex.normal; };
    template<typename T> concept HasUv = requires(T vertex) { vertex.uv; };

    template<class Mat4X4, class RotationAngles, class MeshTypeTrait, class VertType = Vertex<>,
             class VboType = std::vector<VertType>, class EboType = std::vector<Vector3<std::uint32_t>>>
    class Mesh final
    {
    public:
        using VectorType = VertType::VectorType;
        using VertexType = VboType::value_type;

    private:
        using Vbo = VboType;
        using Ebo = EboType;

    public:
        Vbo m_vertex_buffer;
        Ebo m_element_buffer_object;

        Mesh(Vbo vbo, Ebo vao,
             const VectorType scale =
                     {
                             1,
                             1,
                             1,
                     })
            : m_vertex_buffer(std::move(vbo)), m_element_buffer_object(std::move(vao)), m_scale(std::move(scale))
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
            m_to_world_matrix = mat_translation<float, Mat4X4::get_store_ordering()>(m_origin)
                                * MeshTypeTrait::rotation_matrix(m_rotation_angles)
                                * mat_scale<float, Mat4X4::get_store_ordering()>(m_scale);

            return m_to_world_matrix.value();
        }

        [[nodiscard]]
        VectorType vertex_position_to_world_space(const Vector3<float>& vertex_position) const
        {
            auto abs_vec = get_to_world_matrix()
                           * mat_column_from_vector<typename Mat4X4::ContainedType, Mat4X4::get_store_ordering()>(
                                   vertex_position);

            return {abs_vec.at(0, 0), abs_vec.at(1, 0), abs_vec.at(2, 0)};
        }

        // Axis-aligned bounds in the mesh's own frame. No transform is involved, so this is a plain min/max sweep of
        // the vertex buffer, and the result only changes when the vertex buffer does. Intended as a broadphase
        // reject in front of a face-by-face trace: hoist it out of the query loop and slab-test against it, rather
        // than paying the sweep per trace.
        [[nodiscard]]
        Aabb<typename VectorType::ContainedType> local_bounds() const
        {
            using Scalar = VectorType::ContainedType;

            VectorType min{std::numeric_limits<Scalar>::max(), std::numeric_limits<Scalar>::max(),
                           std::numeric_limits<Scalar>::max()};
            VectorType max{std::numeric_limits<Scalar>::lowest(), std::numeric_limits<Scalar>::lowest(),
                           std::numeric_limits<Scalar>::lowest()};

            for (const auto& vertex : m_vertex_buffer)
            {
                const auto& position = [&vertex]() -> const VectorType&
                {
                    if constexpr (HasPosition<VertexType>)
                        return vertex.position;
                    else
                        return vertex;
                }();

                min.x = std::min(min.x, position.x);
                min.y = std::min(min.y, position.y);
                min.z = std::min(min.z, position.z);
                max.x = std::max(max.x, position.x);
                max.y = std::max(max.y, position.y);
                max.z = std::max(max.z, position.z);
            }

            return {min, max};
        }

        [[nodiscard]]
        Triangle<VectorType> make_face_in_local_space(const Ebo::const_iterator vao_iterator) const
        {
            if constexpr (HasPosition<VertexType>)
            {
                return {m_vertex_buffer.at(vao_iterator->x).position, m_vertex_buffer.at(vao_iterator->y).position,
                        m_vertex_buffer.at(vao_iterator->z).position};
            }
            else
            {
                return {m_vertex_buffer.at(vao_iterator->x), m_vertex_buffer.at(vao_iterator->y),
                        m_vertex_buffer.at(vao_iterator->z)};
            }
        }

        [[nodiscard]]
        Triangle<VectorType> make_face_in_world_space(const Ebo::const_iterator vao_iterator) const
        {
            const auto face = make_face_in_local_space(vao_iterator);

            return {vertex_position_to_world_space(face.m_vertex1), vertex_position_to_world_space(face.m_vertex2),
                    vertex_position_to_world_space(face.m_vertex3)};
        }

    private:
        VectorType m_origin;
        VectorType m_scale;

        RotationAngles m_rotation_angles;

        mutable std::optional<Mat4X4> m_to_world_matrix;
    };
} // namespace omath::primitives