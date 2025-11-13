#pragma once
#include "simplex.hpp"
#include <algorithm> // find_if
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <queue>
#include <vector>

namespace omath::collision
{
    template<class V>
    concept EpaVector = requires(const V& a, const V& b, float s)
    {
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
        using Vertex = typename ColliderType::VertexType;
        static_assert(EpaVector<Vertex>, "VertexType must satisfy EpaVector concept");

        struct Result
        {
            bool success{false};
            Vertex normal{}; // outward normal (from B to A)
            float depth{0.0f};
            int iterations{0};
            int num_vertices{0};
            int num_faces{0};
        };

        struct Params
        {
            int max_iterations{64};
            float tolerance{1e-4f}; // absolute tolerance on distance growth
        };

        // Precondition: simplex.size()==4 and contains the origin.
        [[nodiscard]]
        static Result solve(const ColliderType& a, const ColliderType& b, const Simplex<Vertex>& simplex,
                            const Params params = {})
        {
            // --- Build initial polytope from simplex (4 points) ---
            std::vector<Vertex> verts;
            verts.reserve(64);
            for (std::size_t i = 0; i < simplex.size(); ++i)
                verts.push_back(simplex[i]);

            // Initial tetra faces (windings corrected in make_face)
            std::vector<Face> faces;
            faces.reserve(128);
            faces.emplace_back(make_face(verts, 0, 1, 2));
            faces.emplace_back(make_face(verts, 0, 2, 3));
            faces.emplace_back(make_face(verts, 0, 3, 1));
            faces.emplace_back(make_face(verts, 1, 3, 2));

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

                // Get farthest point in face normal direction
                const Vertex p = support_point(a, b, f.n);
                const float p_dist = f.n.dot(p);

                // Converged if we can’t push the face closer than tolerance
                if (p_dist - f.d <= params.tolerance)
                {
                    out.success = true;
                    out.normal = f.n;
                    out.depth = f.d; // along unit normal
                    out.iterations = it + 1;
                    out.num_vertices = static_cast<int>(verts.size());
                    out.num_faces = static_cast<int>(faces.size());
                    return out;
                }

                // Add new vertex
                const int new_idx = static_cast<int>(verts.size());
                verts.push_back(p);

                // Mark faces visible from p and collect their horizon
                std::vector<char> to_delete(faces.size(), 0);
                std::vector<Edge> boundary;
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
                std::vector<Face> new_faces;
                new_faces.reserve(faces.size() + boundary.size());
                for (int i = 0; i < static_cast<int>(faces.size()); ++i)
                    if (!to_delete[i])
                        new_faces.push_back(faces[i]);
                faces.swap(new_faces);

                // Stitch new faces around the horizon
                for (const auto& e : boundary)
                    faces.push_back(make_face(verts, e.a, e.b, new_idx));

                // Rebuild heap after topology change
                heap = rebuild_heap(faces);

                if (!std::isfinite(verts.back().dot(verts.back())))
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
                out.success = true;
                out.normal = best.n;
                out.depth = best.d;
                out.num_vertices = static_cast<int>(verts.size());
                out.num_faces = static_cast<int>(faces.size());
            }
            return out;
        }

    private:
        struct Face
        {
            int i0, i1, i2;
            Vertex n; // unit outward normal
            float d; // n · v0  (>=0 ideally because origin is inside)
        };

        struct Edge
        {
            int a, b;
        };

        struct HeapItem
        {
            float d;
            int idx;
        };
        struct HeapCmp
        {
            bool operator()(const HeapItem& lhs, const HeapItem& rhs) const noexcept
            {
                return lhs.d > rhs.d; // min-heap by distance
            }
        };
        using Heap = std::priority_queue<HeapItem, std::vector<HeapItem>, HeapCmp>;

        static Heap rebuild_heap(const std::vector<Face>& faces)
        {
            Heap h;
            for (int i = 0; i < static_cast<int>(faces.size()); ++i)
                h.push({faces[i].d, i});
            return h;
        }

        static bool visible_from(const Face& f, const Vertex& p)
        {
            // positive if p is in front of the face
            return (f.n.dot(p) - f.d) > 1e-7f;
        }

        static void add_edge_boundary(std::vector<Edge>& boundary, int a, int b)
        {
            // Keep edges that appear only once; erase if opposite already present
            auto itb =
                    std::find_if(boundary.begin(), boundary.end(), [&](const Edge& e) { return e.a == b && e.b == a; });
            if (itb != boundary.end())
                boundary.erase(itb); // internal edge cancels out
            else
                boundary.push_back({a, b}); // horizon edge (directed)
        }

        static Face make_face(const std::vector<Vertex>& verts, int i0, int i1, int i2)
        {
            const Vertex& a0 = verts[i0];
            const Vertex& a1 = verts[i1];
            const Vertex& a2 = verts[i2];
            Vertex n = (a1 - a0).cross(a2 - a0);
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

        static Vertex support_point(const ColliderType& a, const ColliderType& b, const Vertex& dir)
        {
            return a.find_abs_furthest_vertex(dir) - b.find_abs_furthest_vertex(-dir);
        }

        template<class V>
        static constexpr bool near_zero_vec(const V& v, const float eps = 1e-7f)
        {
            return v.dot(v) <= eps * eps;
        }

        template<class V>
        static constexpr V any_perp_vec(const V& v)
        {
            for (const auto& dir : {V{1, 0, 0}, V{0, 1, 0}, V{0, 0, 1}})
                if (const auto d = v.cross(dir); !near_zero_vec(d))
                    return d;
            return V{1, 0, 0};
        }
    };

    // Optional: the GJK that returns a simplex for EPA (unchanged)
    template<class ColliderType>
    class GjkAlgorithmWithSimplex final
    {
        using Vertex = ColliderType::VertexType;

    public:
        struct Hit
        {
            bool hit{false};
            Simplex<Vertex> simplex;
        };

        [[nodiscard]]
        static Vertex find_support_vertex(const ColliderType& a, const ColliderType& b, const Vertex& dir)
        {
            return a.find_abs_furthest_vertex(dir) - b.find_abs_furthest_vertex(-dir);
        }

        [[nodiscard]]
        static Hit collide(const ColliderType& a, const ColliderType& b)
        {
            auto support = find_support_vertex(a, b, {1, 0, 0});
            Simplex<Vertex> simplex;
            simplex.push_front(support);
            auto direction = -support;

            while (true)
            {
                support = find_support_vertex(a, b, direction);
                if (support.dot(direction) <= 0.f)
                    return {};
                simplex.push_front(support);
                if (simplex.handle(direction))
                {
                    if (simplex.size() == 4)
                        return {true, simplex};
                    // rare degeneracy: reseed
                    support = find_support_vertex(a, b, {0, 1, 0});
                    simplex.clear();
                    simplex.push_front(support);
                    direction = -support;
                }
            }
        }
    };
} // namespace omath::collision
