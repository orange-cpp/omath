# `omath::pathfinding::NavigationMesh` — Lightweight vertex graph for A*

> Header: your project’s `pathfinding/navigation_mesh.hpp`
> Namespace: `omath::pathfinding`
> Nodes: `Vector3<float>` (3D points)
> Storage: adjacency map `unordered_map<Vector3<float>, std::vector<Vector3<float>>>`

A minimal navigation mesh represented as a **vertex/edge graph**. Each vertex is a `Vector3<float>` and neighbors are stored in an adjacency list. Designed to pair with `Astar::find_path`.

---

## API

```cpp
class NavigationMesh final {
public:
  // Nearest graph vertex to an arbitrary 3D point.
  // On success -> closest vertex; on failure -> std::string error (e.g., empty mesh).
  [[nodiscard]]
  std::expected<Vector3<float>, std::string>
  get_closest_vertex(const Vector3<float>& point) const noexcept;

  // Read-only neighbor list for a vertex key.
  // If vertex is absent, implementation should return an empty list (see notes).
  [[nodiscard]]
  const std::vector<Vector3<float>>&
  get_neighbors(const Vector3<float>& vertex) const noexcept;

  // True if the graph has no vertices/edges.
  [[nodiscard]]
  bool empty() const;

  // Serialize/deserialize the graph (opaque binary).
  [[nodiscard]] std::vector<uint8_t> serialize() const noexcept;
  void deserialize(const std::vector<uint8_t>& raw) noexcept;

  // Public adjacency (vertex -> neighbors)
  std::unordered_map<Vector3<float>, std::vector<Vector3<float>>> m_vertex_map;
};
```

---

## Quick start

```cpp
using omath::Vector3;
using omath::pathfinding::NavigationMesh;

// Build a tiny mesh (triangle)
NavigationMesh nav;
nav.m_vertex_map[ {0,0,0} ] = { {1,0,0}, {0,0,1} };
nav.m_vertex_map[ {1,0,0} ] = { {0,0,0}, {0,0,1} };
nav.m_vertex_map[ {0,0,1} ] = { {0,0,0}, {1,0,0} };

// Query the closest node to an arbitrary point
auto q = nav.get_closest_vertex({0.3f, 0.0f, 0.2f});
if (q) {
  const auto& v = *q;
  const auto& nbrs = nav.get_neighbors(v);
  (void)nbrs;
}
```

---

## Semantics & expectations

* **Nearest vertex**
  `get_closest_vertex(p)` should scan known vertices and return the one with minimal Euclidean distance to `p`. If the mesh is empty, expect an error (`unexpected` with a message).

* **Neighbors**
  `get_neighbors(v)` returns the adjacency list for `v`. If `v` is not present, a conventional behavior is to return a **reference to a static empty vector** (since the API is `noexcept` and returns by reference). Verify in your implementation.

* **Graph invariants** (recommended)

    * Neighbor links are **symmetric** for undirected navigation (if `u` has `v`, then `v` has `u`).
    * No self-loops unless explicitly desired.
    * Vertices are unique keys; hashing uses `std::hash<Vector3<float>>` (be mindful of floating-point equality).

---

## Serialization

* `serialize()` → opaque, implementation-defined binary of the current `m_vertex_map`.
* `deserialize(raw)` → restores the internal map from `raw`.
  Keep versioning in mind if you evolve the format (e.g., add a header/magic/version).

---

## Performance

Let `V = m_vertex_map.size()` and `E = Σ|neighbors(v)|`.

* `get_closest_vertex`: **O(V)** (linear scan) unless you back it with a spatial index (KD-tree, grid, etc.).
* `get_neighbors`: **O(1)** average (hash lookup).
* Memory: **O(V + E)**.

---

## Usage notes

* **Floating-point keys**: Using `Vector3<float>` as an unordered_map key relies on your `std::hash<omath::Vector3<float>>` and exact `operator==`. Avoid building meshes with numerically “close but not equal” duplicates; consider canonicalizing or using integer IDs if needed.
* **Pathfinding**: Pair with `Astar::find_path(start, end, nav)`; the A* heuristic can use straight-line distance between vertex positions.

---

## Minimal test ideas

* Empty mesh → `get_closest_vertex` returns error; `empty() == true`.
* Single vertex → nearest always that vertex; neighbors empty.
* Symmetric edges → `get_neighbors(a)` contains `b` and vice versa.
* Serialization round-trip preserves vertex/edge counts and neighbor lists.
