# `omath::pathfinding::Astar` — Pathfinding over a navigation mesh

> Header: your project’s `pathfinding/astar.hpp`
> Namespace: `omath::pathfinding`
> Inputs: start/end as `Vector3<float>`, a `NavigationMesh`
> Output: ordered list of waypoints `std::vector<Vector3<float>>`

`Astar` exposes a single public function, `find_path`, that computes a path of 3D waypoints on a navigation mesh. Internally it reconstructs the result with `reconstruct_final_path` from a closed set keyed by `Vector3<float>`.

---

## API

```cpp
namespace omath::pathfinding {

struct PathNode; // holds per-node search data (see "Expected PathNode fields")

class Astar final {
public:
  [[nodiscard]]
  static std::vector<Vector3<float>>
  find_path(const Vector3<float>& start,
            const Vector3<float>& end,
            const NavigationMesh& nav_mesh) noexcept;

private:
  [[nodiscard]]
  static std::vector<Vector3<float>>
  reconstruct_final_path(
    const std::unordered_map<Vector3<float>, PathNode>& closed_list,
    const Vector3<float>& current) noexcept;
};

} // namespace omath::pathfinding
```

### Semantics

* Returns a **polyline** of 3D points from `start` to `end`.
* If no path exists, the function typically returns an **empty vector** (behavior depends on implementation details; keep this contract in unit tests).

---

## What `NavigationMesh` is expected to provide

The header doesn’t constrain `NavigationMesh`, but for A* it commonly needs:

* **Neighborhood queries**: given a position or node key → iterable neighbors.
* **Traversal cost**: `g(u,v)` (often Euclidean distance or edge weight).
* **Heuristic**: `h(x,end)` (commonly straight-line distance on the mesh).
* **Projection / snap**: the ability to map `start`/`end` to valid nodes/points on the mesh (if they are off-mesh).

> If your `NavigationMesh` doesn’t directly expose these, `Astar::find_path` likely does the adapter work (snapping to the nearest convex polygon/portal nodes and expanding across adjacency).

---

## Expected `PathNode` fields

Although not visible here, `PathNode` typically carries:

* `Vector3<float> parent;` — predecessor position or key for backtracking
* `float g;` — cost from `start`
* `float h;` — heuristic to `end`
* `float f;` — `g + h`

`reconstruct_final_path(closed_list, current)` walks `parent` links from `current` back to the start, **reverses** the chain, and returns the path.

---

## Heuristic & optimality

* Use an **admissible** heuristic (never overestimates true cost) to keep A* optimal.
  The usual choice is **Euclidean distance** in 3D:

  ```cpp
  h(x, goal) = (goal - x).length();
  ```
* For best performance, make it **consistent** (triangle inequality holds). Euclidean distance is consistent on standard navmeshes.

---

## Complexity

Let `V` be explored vertices (or portal nodes) and `E` the traversed edges.

* With a binary heap open list: **O(E log V)** time, **O(V)** memory.
* With a d-ary heap or pairing heap you may reduce practical constants.

---

## Typical usage

```cpp
#include "omath/pathfinding/astar.hpp"
#include "omath/pathfinding/navigation_mesh.hpp"

using omath::Vector3;
using omath::pathfinding::Astar;

NavigationMesh nav = /* ... load/build mesh ... */;

Vector3<float> start{2.5f, 0.0f, -1.0f};
Vector3<float> goal {40.0f, 0.0f, 12.0f};

auto path = Astar::find_path(start, goal, nav);

if (!path.empty()) {
  // feed to your agent/renderer
  for (const auto& p : path) {
    // draw waypoint p or push to steering
  }
} else {
  // handle "no path" (e.g., unreachable or disconnected mesh)
}
```

---

## Notes & recommendations

* **Start/end snapping**: If `start` or `end` are outside the mesh, decide whether to snap to the nearest polygon/portal or fail early. Keep this behavior consistent and document it where `NavigationMesh` is defined.
* **Numerical stability**: Prefer squared distances when only comparing (`dist2`) to avoid unnecessary `sqrt`.
* **Tie-breaking**: When `f` ties are common (grid-like graphs), bias toward larger `g` or smaller `h` to reduce zig-zagging.
* **Smoothing**: A* returns a polyline that may hug polygon edges. Consider:

    * **String pulling / Funnel algorithm** over the corridor of polygons to get a straightened path.
    * **Raycast smoothing** (visibility checks) to remove redundant interior points.
* **Hashing `Vector3<float>`**: Your repo defines `std::hash<omath::Vector3<float>>`. Ensure equality/precision rules for using float keys are acceptable (or use discrete node IDs instead).

---

## Testing checklist

* Start/end on the **same polygon** → direct path of 2 points.
* **Disconnected components** → empty result.
* **Narrow corridors** → path stays inside.
* **Obstacles blocking** → no path.
* **Floating-point noise** → still reconstructs a valid chain from parents.

---

## Minimal pseudo-implementation outline (for reference)

```cpp
// Pseudocode only — matches the header’s intent
std::vector<Vec3> find_path(start, goal, mesh) {
  auto [snode, gnode] = mesh.snap_to_nodes(start, goal);
  OpenSet open; // min-heap by f
  std::unordered_map<Vec3, PathNode> closed;

  open.push({snode, g=0, h=heuristic(snode, gnode)});
  parents.clear();

  while (!open.empty()) {
    auto current = open.pop_min(); // node with lowest f

    if (current.pos == gnode.pos)
      return reconstruct_final_path(closed, current.pos);

    for (auto [nbr, cost] : mesh.neighbors(current.pos)) {
      float tentative_g = current.g + cost;
      if (auto it = closed.find(nbr); it == closed.end() || tentative_g < it->second.g) {
        closed[nbr] = { .parent = current.pos,
                        .g = tentative_g,
                        .h = heuristic(nbr, gnode.pos),
                        .f = tentative_g + heuristic(nbr, gnode.pos) };
        open.push(closed[nbr]);
      }
    }
  }
  return {}; // no path
}
```

---

## FAQ

* **Why return `std::vector<Vector3<float>>` and not polygon IDs?**
  Waypoints are directly usable by agents/steering and for rendering. If you also need the corridor (polygon chain), extend the API or `PathNode` to store it.

* **Does `find_path` modify the mesh?**
  No; it should be a read-only search over `NavigationMesh`.

---

*Last updated: 24 Dec 2025*
