//
// Created by vlad on 11/15/2024.
//
#pragma once
#include "ICollidable.h"

namespace omath::collision
{
    class Cube final : public ICollidable
    {
    public:

        [[nodiscard]]
        bool IsCollideWith(const std::shared_ptr<ICollidable>& other) override;

    private:
        [[nodiscard]]
        bool IsCollideWithCube(const Cube& other);
        bool IsCollideWithCapsule(const Cube& other);

    };
}