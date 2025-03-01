//
// Created by Vlad on 28.07.2024.
//
#include "omath/pathfinding/Astar.hpp"

#include <algorithm>
#include <optional>
#include <unordered_map>
#include <unordered_set>


namespace omath::pathfinding
{
    struct PathNode final
    {
        std::optional<Vector3<float>> cameFrom;
        float gCost = 0.f;
    };


    std::vector<Vector3<float>> Astar::ReconstructFinalPath(const std::unordered_map<Vector3<float>, PathNode>& closedList,
                                                     const Vector3<float>& current)
    {
        std::vector<Vector3<float>> path;
        std::optional currentOpt = current;

        while (currentOpt)
        {
            path.push_back(*currentOpt);

            auto it = closedList.find(*currentOpt);

            if (it == closedList.end())
                break;

            currentOpt = it->second.cameFrom;
        }

        std::ranges::reverse(path);
        return path;
    }
    auto Astar::GetPerfectNode(const std::unordered_map<Vector3<float>, PathNode>& openList, const Vector3<float>& endVertex)
    {
        return std::ranges::min_element(openList,
                                        [&endVertex](const auto& a, const auto& b)
                                        {
                                            const float fA = a.second.gCost + a.first.DistTo(endVertex);
                                            const float fB = b.second.gCost + b.first.DistTo(endVertex);
                                            return fA < fB;
                                        });
    }

    std::vector<Vector3<float>> Astar::FindPath(const Vector3<float>& start, const Vector3<float>& end, const NavigationMesh& navMesh)
    {
        std::unordered_map<Vector3<float>, PathNode> closedList;
        std::unordered_map<Vector3<float>, PathNode> openList;

        auto maybeStartVertex = navMesh.GetClosestVertex(start);
        auto maybeEndVertex = navMesh.GetClosestVertex(end);

        if (!maybeStartVertex || !maybeEndVertex)
            return {};

        const auto startVertex = maybeStartVertex.value();
        const auto endVertex = maybeEndVertex.value();


        openList.emplace(startVertex, PathNode{std::nullopt, 0.f});

        while (!openList.empty())
        {
            auto currentIt = GetPerfectNode(openList, endVertex);

            const auto current = currentIt->first;
            const auto currentNode = currentIt->second;

            if (current == endVertex)
                return ReconstructFinalPath(closedList, current);


            closedList.emplace(current, currentNode);
            openList.erase(currentIt);

            for (const auto& neighbor: navMesh.GetNeighbors(current))
            {
                if (closedList.contains(neighbor))
                    continue;

                const float tentativeGCost = currentNode.gCost + neighbor.DistTo(current);

                const auto openIt = openList.find(neighbor);

                if (openIt == openList.end() || tentativeGCost < openIt->second.gCost)
                    openList[neighbor] = PathNode{current, tentativeGCost};
            }
        }

        return {};
    }
} // namespace omath::pathfinding
