//
// Created by Vlad on 28.07.2024.
//
#include "omath/pathfinding/Astar.h"
#include <unordered_map>



namespace omath::pathfinding
{
    struct PathNode final
    {
        PathNode* cameFrom;
        const NavigationVertex* navVertex;
        float gCost = 0.f;
    };


    std::vector<Vector3> Astar::FindPath(const Vector3 &start, const Vector3 &end, const NavigationMesh &navMesh)
    {
        std::unordered_map<Vector3, PathNode> closedList;
        std::unordered_map<Vector3, PathNode> openList;

        const auto& startVertex = navMesh.GetClossestVertex(start).value();
        const auto& endVertex = navMesh.GetClossestVertex(end).value();

        openList.emplace(startVertex.origin, PathNode{nullptr, &startVertex, 0.f});

        while (!openList.empty())
        {
            
        }

        return {};
    }
}
