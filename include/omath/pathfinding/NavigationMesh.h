//
// Created by Vlad on 28.07.2024.
//

#pragma once

#include "omath/Vector3.h"
#include <expected>
#include <vector>
#include <string>


namespace omath::pathfinding
{
    struct NavigationVertex
    {
        Vector3 origin;
        std::vector<Vector3*> connections;
    };


    class NavigationMesh final
    {
    public:

        [[nodiscard]]
        std::expected<const NavigationVertex, std::string> GetClossestVertex(const Vector3& point) const;

    private:


    };
}