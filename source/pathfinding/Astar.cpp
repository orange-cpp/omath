//
// Created by Vlad on 28.07.2024.
//
#include "omath/pathfinding/Astar.h"

#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>


namespace omath::pathfinding
{
    struct PathNode final
    {
        std::optional<Vector3> cameFrom;
        float gCost = 0.f;
    };


    std::vector<Vector3> Astar::FindPath(const Vector3 &start, const Vector3 &end, const NavigationMesh &navMesh)
    {
        std::unordered_map<Vector3, PathNode> closedList;
        std::unordered_map<Vector3, PathNode> openList;

        const auto startVertex = navMesh.GetClosestVertex(start).value();
        const auto endVertex = navMesh.GetClosestVertex(end).value();

        openList.emplace(startVertex, PathNode{std::nullopt, 0.f});

        while (!openList.empty())
        {
            const auto perfectVertex = *std::ranges::min_element(openList,
                [&endVertex](const auto& a, const auto& b) -> bool
                {
                    const auto aCost = a.second.gCost + a.first.DistTo(endVertex);
                    const auto bCost = b.second.gCost + b.first.DistTo(endVertex);
                    return aCost < bCost;
                });

            closedList.emplace(perfectVertex);
            openList.erase(perfectVertex.first);

            for (const auto& neighbor : navMesh.GetNeighbors(perfectVertex.first))
                if (!closedList.contains(neighbor))
                    openList.emplace(neighbor, PathNode{perfectVertex.first, neighbor.DistTo(perfectVertex.first) + perfectVertex.second.gCost});


            if (perfectVertex.first != endVertex)
                continue;

            std::vector<Vector3> path = {};

            for (std::optional current = perfectVertex.first; current; current = closedList.at(*current).cameFrom )
                path.push_back(current.value());

            return path;
        }

        return {};
    }
}
