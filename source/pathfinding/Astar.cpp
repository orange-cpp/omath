//
// Created by Vlad on 28.07.2024.
//
#include "omath/pathfinding/Astar.h"
#include <unordered_map>
#include <unordered_set>


namespace omath::pathfinding
{
    struct PathNode final
    {
        Vector3 cameFrom;
        NavigationVertex const* navVertex;
        float gCost = 0.f;
    };


    std::vector<Vector3> Astar::FindPath(const Vector3 &start, const Vector3 &end, const NavigationMesh &navMesh)
    {
        std::unordered_map<Vector3, PathNode> closedList;
        std::unordered_map<Vector3, PathNode> openList;

        const auto& startVertex = navMesh.GetClossestVertex(start).value();
        const auto& endVertex = navMesh.GetClossestVertex(end).value();

        openList.emplace(startVertex.origin, PathNode{startVertex.origin, &startVertex, 0.f});

        while (!openList.empty())
        {
            const auto [cord, node] = *std::ranges::min_element(openList,
                [&endVertex](const auto& a, const auto& b) -> bool
                {
                    const auto aCost = a.second.gCost + a.second.navVertex->origin.DistTo(endVertex.origin);
                    const auto bCost = b.second.gCost + b.second.navVertex->origin.DistTo(endVertex.origin);
                    return aCost < bCost;
                });

            openList.erase(cord);

            for (const auto& neighbor : node.navVertex->connections)
                if (!closedList.contains(neighbor->origin))
                    closedList.emplace(neighbor->origin, PathNode{cord, neighbor, neighbor->origin.DistTo(cord) + node.gCost});
        }

        return {};
    }
}
