//
// Created by Vladislav on 06.12.2025.
//
#pragma once


namespace omath::collision
{
    template<class VecType = Vector3<float>>
    class ColliderInterface
    {
    public:
        using VectorType = VecType;
        virtual ~ColliderInterface() = default;

        [[nodiscard]]
        virtual VectorType find_abs_furthest_vertex_position(const VectorType& direction) const = 0;

        [[nodiscard]]
        virtual const VectorType& get_origin() const = 0;
        virtual void set_origin(const VectorType& new_origin) = 0;
    };
}