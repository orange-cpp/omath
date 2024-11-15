//
// Created by vlad on 11/15/2024.
//
#pragma once
#include "ICollidable.h"
#include <memory>



namespace omath::collision
{
    class ICollidable
    {
        public:
        virtual ~ICollidable() = default;

        [[nodiscard]]
        virtual bool IsCollideWith(const std::shared_ptr<ICollidable>& other) = 0;
    };
}
