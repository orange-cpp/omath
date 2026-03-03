#pragma once
#include "simplex.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <memory_resource>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

namespace omath::collision
{
    template<class V, class FloatingType>
    concept EpaVector = requires(const V& a, const V& b, FloatingType s) {
        { a - b } -> std::same_as<V>;
        { a.cross(b) } -> std::same_as<V>;
        { a.dot(b) } -> std::same_as<FloatingType>;
        { -a } -> std::same_as<V>;
        { a * s } -> std::same_as<V>;
        { a / s } -> std::same_as<V>;
    };

    template<class ColliderInterfaceType>
    class Epa final
    {
    public:
        using VectorType = ColliderInterfaceType::VectorType;
        static_assert(EpaVector<VectorType, typename VectorType::ContainedType>,
                      "VertexType must satisfy EpaVector concept");

    private:
        using FloatingType = VectorType::ContainedType;

    public:
        struct Result final
        {
            VectorType normal{}; // from A to B
            VectorType penetration_vector;
            FloatingType depth{0.0};
            int iterations{0};
            int num_vertices{0};
            int num_faces{0};
        };

        struct Params final
        {
            int max_iterations{64};
            FloatingType tolerance{1e-4}; // absolute tolerance on distance growth
        };
        // Precondition: simplex.size()==4 and contains the origin.
        [[nodiscard]]
        static std::optional<Result> solve(const ColliderInterfaceType& a, const ColliderInterfaceType& b,
                                           const Simplex<VectorType>& simplex, const Params params = {},
                                           std::pmr::memory_resource& mem_resource = *std::pmr::get_default_resource())
        {
            std::pmr::vector<VectorType> vertexes = build_initial_polytope_from_simplex(simplex, mem_resource);
            std::pmr::vector<Face> faces = create_initial_tetra_faces(mem_resource, vertexes);

            // Build initial min-heap by distance.
            Heap heap = rebuild_heap(faces, mem_resource);

            Result out{};

            // Hoisted outside the loop to reuse bucket allocation across iterations.
            // Initial bucket count 16 covers a typical horizon without rehashing.
            BoundaryMap boundary{16, &mem_resource};

            for (int it = 0; it < params.max_iterations; ++it)
            {
                // Lazily discard stale (deleted or index-mismatched) heap entries.
                discard_stale_heap_entries(faces, heap);

                if (heap.empty())
                    break;

                const Face& face = faces[heap.top().idx];

                const VectorType p = support_point(a, b, face.n);
                const auto p_dist = face.n.dot(p);

                // Converged: new support can't push the face closer than tolerance.
                if (p_dist - face.d <= params.tolerance)
                {
                    out.normal = face.n;
                    out.depth = face.d;
                    out.iterations = it + 1;
                    out.num_vertices = static_cast<int>(vertexes.size());
                    out.num_faces = static_cast<int>(faces.size());
                    out.penetration_vector = out.normal * out.depth;
                    return out;
                }

                const int new_idx = static_cast<int>(vertexes.size());
                vertexes.emplace_back(p);

                // Tombstone visible faces and collect the horizon boundary.
                // This avoids copying the faces array (O(n)) each iteration.
                tombstone_visible_faces(faces, boundary, p);

                // Stitch new faces around the horizon and push them directly onto the
                // heap — no full O(n log n) rebuild needed.
                for (const auto& [key, e] : boundary)
                {
                    const int fi = static_cast<int>(faces.size());
                    faces.emplace_back(make_face(vertexes, e.a, e.b, new_idx));
                    heap.emplace(faces.back().d, fi);
                }

                if (!std::isfinite(vertexes.back().dot(vertexes.back())))
                    break; // safety

                out.iterations = it + 1;
            }

            // Find the best surviving (non-deleted) face.
            const Face* best = find_best_surviving_face(faces);

            if (!best)
                return std::nullopt;

            out.normal = best->n;
            out.depth = best->d;
            out.num_vertices = static_cast<int>(vertexes.size());
            out.num_faces = static_cast<int>(faces.size());
            out.penetration_vector = out.normal * out.depth;
            return out;
        }

    private:
        struct Face final
        {
            int i0, i1, i2;
            VectorType n; // unit outward normal
            FloatingType d; // n · v0  (>= 0 ideally because origin is inside)
            bool deleted{false}; // tombstone flag — avoids O(n) compaction per iteration
        };

        struct Edge final
        {
            int a, b;
        };

        struct HeapItem final
        {
            FloatingType d;
            int idx;
        };

        struct HeapCmp final
        {
            [[nodiscard]]
            static bool operator()(const HeapItem& lhs, const HeapItem& rhs) noexcept
            {
                return lhs.d > rhs.d; // min-heap by distance
            }
        };

        using Heap = std::priority_queue<HeapItem, std::pmr::vector<HeapItem>, HeapCmp>;

        // Horizon boundary: maps packed(a,b) → Edge.
        // Opposite edges cancel in O(1) via hash lookup instead of O(h) linear scan.
        using BoundaryMap = std::pmr::unordered_map<int64_t, Edge>;

        [[nodiscard]]
        static constexpr int64_t pack_edge(int a, int b) noexcept
        {
            return (static_cast<int64_t>(a) << 32) | static_cast<uint32_t>(b);
        }

        [[nodiscard]]
        static Heap rebuild_heap(const std::pmr::vector<Face>& faces, auto& memory_resource)
        {
            std::pmr::vector<HeapItem> storage{&memory_resource};
            storage.reserve(faces.size());
            Heap h{HeapCmp{}, std::move(storage)};
            for (int i = 0; i < static_cast<int>(faces.size()); ++i)
                if (!faces[i].deleted)
                    h.emplace(faces[i].d, i);
            return h;
        }

        [[nodiscard]]
        static bool visible_from(const Face& f, const VectorType& p)
        {
            return f.n.dot(p) - f.d > static_cast<FloatingType>(1e-7);
        }

        static void add_edge_boundary(BoundaryMap& boundary, int a, int b)
        {
            // O(1) cancel: if the opposite edge (b→a) is already in the map it is an
            // internal edge shared by two visible faces and must be removed.
            // Otherwise this is a horizon edge and we insert it.
            const int64_t rev = pack_edge(b, a);
            if (const auto it = boundary.find(rev); it != boundary.end())
                boundary.erase(it);
            else
                boundary.emplace(pack_edge(a, b), Edge{a, b});
        }

        [[nodiscard]]
        static Face make_face(const std::pmr::vector<VectorType>& vertexes, int i0, int i1, int i2)
        {
            const VectorType& a0 = vertexes[i0];
            const VectorType& a1 = vertexes[i1];
            const VectorType& a2 = vertexes[i2];
            VectorType n = (a1 - a0).cross(a2 - a0);
            if (n.dot(n) <= static_cast<FloatingType>(1e-30))
                n = any_perp_vec(a1 - a0); // degenerate guard
            // Ensure normal points outward (away from origin): require n·a0 >= 0
            if (n.dot(a0) < static_cast<FloatingType>(0.0))
            {
                std::swap(i1, i2);
                n = -n;
            }
            const auto inv_len =
                    static_cast<FloatingType>(1.0) / std::sqrt(std::max(n.dot(n), static_cast<FloatingType>(1e-30)));
            n = n * inv_len;
            const auto d = n.dot(a0);
            return {i0, i1, i2, n, d};
        }

        [[nodiscard]]
        static VectorType support_point(const ColliderInterfaceType& a, const ColliderInterfaceType& b,
                                        const VectorType& dir)
        {
            return a.find_abs_furthest_vertex_position(dir) - b.find_abs_furthest_vertex_position(-dir);
        }

        template<class V>
        [[nodiscard]]
        static constexpr bool near_zero_vec(const V& v, const FloatingType eps = 1e-7f)
        {
            return v.dot(v) <= eps * eps;
        }

        template<class V>
        [[nodiscard]]
        static constexpr V any_perp_vec(const V& v)
        {
            for (const auto& dir : {V{1, 0, 0}, V{0, 1, 0}, V{0, 0, 1}})
                if (const auto d = v.cross(dir); !near_zero_vec(d))
                    return d;
            return V{1, 0, 0};
        }

        [[nodiscard]]
        static std::pmr::vector<Face> create_initial_tetra_faces(std::pmr::memory_resource& mem_resource,
                                                                 const std::pmr::vector<VectorType>& vertexes)
        {
            std::pmr::vector<Face> faces{&mem_resource};
            faces.reserve(4);
            faces.emplace_back(make_face(vertexes, 0, 1, 2));
            faces.emplace_back(make_face(vertexes, 0, 2, 3));
            faces.emplace_back(make_face(vertexes, 0, 3, 1));
            faces.emplace_back(make_face(vertexes, 1, 3, 2));
            return faces;
        }

        [[nodiscard]]
        static std::pmr::vector<VectorType> build_initial_polytope_from_simplex(const Simplex<VectorType>& simplex,
                                                                                std::pmr::memory_resource& mem_resource)
        {
            std::pmr::vector<VectorType> vertexes{&mem_resource};
            vertexes.reserve(simplex.size());
            for (std::size_t i = 0; i < simplex.size(); ++i)
                vertexes.emplace_back(simplex[i]);
            return vertexes;
        }

        static const Face* find_best_surviving_face(const std::pmr::vector<Face>& faces)
        {
            const Face* best = nullptr;
            for (const auto& f : faces)
                if (!f.deleted && (best == nullptr || f.d < best->d))
                    best = &f;
            return best;
        }
        static void tombstone_visible_faces(std::pmr::vector<Face>& faces, BoundaryMap& boundary,
                                            const VectorType& p)
        {
            boundary.clear();
            for (auto& f : faces)
            {
                if (!f.deleted && visible_from(f, p))
                {
                    f.deleted = true;
                    add_edge_boundary(boundary, f.i0, f.i1);
                    add_edge_boundary(boundary, f.i1, f.i2);
                    add_edge_boundary(boundary, f.i2, f.i0);
                }
            }
        }

        static void discard_stale_heap_entries(const std::pmr::vector<Face>& faces,
                                               std::priority_queue<HeapItem, std::pmr::vector<HeapItem>, HeapCmp>& heap)
        {
            while (!heap.empty())
            {
                const auto& top = heap.top();
                if (!faces[top.idx].deleted && faces[top.idx].d == top.d)
                    break;
                heap.pop();
            }
        }
    };
} // namespace omath::collision
