#pragma once
#include "simplex.hpp"
#include <algorithm> // find_if
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

namespace omath::collision
{
    template<class V>
    concept EpaVector = requires(const V& a, const V& b, float s) {
        { a - b } -> std::same_as<V>;
        { a.cross(b) } -> std::same_as<V>;
        { a.dot(b) } -> std::same_as<float>;
        { -a } -> std::same_as<V>;
        { a* s } -> std::same_as<V>;
        { a / s } -> std::same_as<V>;
    };

    template<class ColliderType>
    class Epa final
    {
    public:
        using VectorType = ColliderType::VectorType;
        static_assert(EpaVector<VectorType>, "VertexType must satisfy EpaVector concept");

        struct Result final
        {
            VectorType normal{}; // outward normal (from B to A)
            VectorType penetration_vector;
            float depth{0.0f};
            int iterations{0};
            int num_vertices{0};
            int num_faces{0};
        };

        struct Params final
        {
            int max_iterations{64};
            float tolerance{1e-4f}; // absolute tolerance on distance growth
        };

        // Precondition: simplex.size()==4 and contains the origin.
        [[nodiscard]]
        static std::optional<Result> solve(const ColliderType& a, const ColliderType& b,
                                           const Simplex<VectorType>& simplex, const Params params = {},
                                           std::shared_ptr<std::pmr::memory_resource> mem_resource = {
                                                   std::shared_ptr<void>{}, std::pmr::get_default_resource()})
        {
            // --- Build initial polytope from simplex (4 points) ---
            std::pmr::vector<VectorType> vertexes{mem_resource.get()};
            vertexes.reserve(simplex.size());
            for (std::size_t i = 0; i < simplex.size(); ++i)
                vertexes.emplace_back(simplex[i]);

            // Initial tetra faces (windings corrected in make_face)
            std::pmr::vector<Face> faces{mem_resource.get()};
            faces.reserve(4);
            faces.emplace_back(make_face(vertexes, 0, 1, 2));
            faces.emplace_back(make_face(vertexes, 0, 2, 3));
            faces.emplace_back(make_face(vertexes, 0, 3, 1));
            faces.emplace_back(make_face(vertexes, 1, 3, 2));

            auto heap = rebuild_heap(faces);

            Result out{};

            for (int it = 0; it < params.max_iterations; ++it)
            {
                // If heap might be stale after face edits, rebuild lazily.
                if (heap.empty())
                    break;
                // Rebuild when the "closest" face changed (simple cheap guard)
                // (We could keep face handles; this is fine for small Ns.)

                if (const auto top = heap.top(); faces[top.idx].d != top.d)
                    heap = rebuild_heap(faces);

                if (heap.empty())
                    break;

                const int fidx = heap.top().idx;
                const Face f = faces[fidx];

                // Get the furthest point in face normal direction
                const VectorType p = support_point(a, b, f.n);
                const float p_dist = f.n.dot(p);

                // Converged if we can’t push the face closer than tolerance
                if (p_dist - f.d <= params.tolerance)
                {
                    out.normal = f.n;
                    out.depth = f.d; // along unit normal
                    out.iterations = it + 1;
                    out.num_vertices = static_cast<int>(vertexes.size());
                    out.num_faces = static_cast<int>(faces.size());

                    const auto centers = b.get_origin() - a.get_origin();
                    const auto sign = out.normal.dot(centers) >= 0 ? 1 : -1;

                    out.penetration_vector = out.normal * out.depth * sign;
                    return out;
                }

                // Add new vertex
                const int new_idx = static_cast<int>(vertexes.size());
                vertexes.emplace_back(p);

                // Mark faces visible from p and collect their horizon
                std::pmr::vector<char> to_delete(faces.size(), 0, mem_resource.get());
                std::pmr::vector<Edge> boundary{mem_resource.get()};
                boundary.reserve(faces.size() * 2);

                for (int i = 0; i < static_cast<int>(faces.size()); ++i)
                {
                    if (to_delete[i])
                        continue;
                    if (visible_from(faces[i], p))
                    {
                        const auto& rf = faces[i];
                        to_delete[i] = 1;
                        add_edge_boundary(boundary, rf.i0, rf.i1);
                        add_edge_boundary(boundary, rf.i1, rf.i2);
                        add_edge_boundary(boundary, rf.i2, rf.i0);
                    }
                }

                // Remove visible faces
                std::pmr::vector<Face> new_faces{mem_resource.get()};
                new_faces.reserve(faces.size() + boundary.size());
                for (int i = 0; i < static_cast<int>(faces.size()); ++i)
                    if (!to_delete[i])
                        new_faces.emplace_back(faces[i]);
                faces.swap(new_faces);

                // Stitch new faces around the horizon
                for (const auto& e : boundary)
                    faces.emplace_back(make_face(vertexes, e.a, e.b, new_idx));

                // Rebuild heap after topology change
                heap = rebuild_heap(faces);

                if (!std::isfinite(vertexes.back().dot(vertexes.back())))
                    break; // safety
                out.iterations = it + 1;
            }

            // Fallback: pick closest face as best-effort answer
            if (!faces.empty())
            {
                auto best = faces[0];
                for (const auto& f : faces)
                    if (f.d < best.d)
                        best = f;
                out.normal = best.n;
                out.depth = best.d;
                out.num_vertices = static_cast<int>(vertexes.size());
                out.num_faces = static_cast<int>(faces.size());

                const auto centers = b.get_origin() - a.get_origin();
                const auto sign = out.normal.dot(centers) >= 0 ? 1 : -1;

                out.penetration_vector = out.normal * out.depth * sign;

                return out;
            }
            return std::nullopt;
        }

    private:
        struct Face final
        {
            int i0, i1, i2;
            VectorType n; // unit outward normal
            float d; // n · v0  (>=0 ideally because origin is inside)
        };

        struct Edge final
        {
            int a, b;
        };

        struct HeapItem final
        {
            float d;
            int idx;
        };
        struct HeapCmp final
        {
            bool operator()(const HeapItem& lhs, const HeapItem& rhs) const noexcept
            {
                return lhs.d > rhs.d; // min-heap by distance
            }
        };
        using Heap = std::priority_queue<HeapItem, std::vector<HeapItem>, HeapCmp>;

        [[nodiscard]]
        static Heap rebuild_heap(const std::pmr::vector<Face>& faces)
        {
            Heap h;
            for (int i = 0; i < static_cast<int>(faces.size()); ++i)
                h.emplace(faces[i].d, i);
            return h;
        }

        [[nodiscard]]
        static bool visible_from(const Face& f, const VectorType& p)
        {
            // positive if p is in front of the face
            return (f.n.dot(p) - f.d) > 1e-7f;
        }

        static void add_edge_boundary(std::pmr::vector<Edge>& boundary, int a, int b)
        {
            // Keep edges that appear only once; erase if opposite already present
            auto itb =
                    std::find_if(boundary.begin(), boundary.end(), [&](const Edge& e) { return e.a == b && e.b == a; });
            if (itb != boundary.end())
                boundary.erase(itb); // internal edge cancels out
            else
                boundary.emplace_back(a, b); // horizon edge (directed)
        }

        [[nodiscard]]
        static Face make_face(const std::pmr::vector<VectorType>& vertexes, int i0, int i1, int i2)
        {
            const VectorType& a0 = vertexes[i0];
            const VectorType& a1 = vertexes[i1];
            const VectorType& a2 = vertexes[i2];
            VectorType n = (a1 - a0).cross(a2 - a0);
            if (n.dot(n) <= 1e-30f)
            {
                n = any_perp_vec(a1 - a0); // degenerate guard
            }
            // Ensure normal points outward (away from origin): require n·a0 >= 0
            if (n.dot(a0) < 0.0f)
            {
                std::swap(i1, i2);
                n = -n;
            }
            const float inv_len = 1.0f / std::sqrt(std::max(n.dot(n), 1e-30f));
            n = n * inv_len;
            const float d = n.dot(a0);
            return {i0, i1, i2, n, d};
        }

        [[nodiscard]]
        static VectorType support_point(const ColliderType& a, const ColliderType& b, const VectorType& dir)
        {
            return a.find_abs_furthest_vertex(dir).position - b.find_abs_furthest_vertex(-dir).position;
        }

        template<class V>
        [[nodiscard]]
        static constexpr bool near_zero_vec(const V& v, const float eps = 1e-7f)
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
    };
} // namespace omath::collision
