//
// Created by Orange on 04/08/2026.
//

#pragma once
#include "omath/3d_primitives/aabb.hpp"
#include "omath/collision/line_tracer.hpp"
#include <algorithm>
#include <cstdint>
#include <numeric>
#include <span>
#include <vector>

namespace omath::collision
{
    template<class Type = float>
    class BvhTree final
    {
    public:
        using AabbType = primitives::Aabb<Type>;

        struct HitResult
        {
            std::size_t object_index;
            Type distance_sqr;
        };

        BvhTree() = default;

        explicit BvhTree(std::span<const AabbType> aabbs)
            : m_aabbs(aabbs.begin(), aabbs.end())
        {
            if (aabbs.empty())
                return;

            m_indices.resize(aabbs.size());
            std::iota(m_indices.begin(), m_indices.end(), std::size_t{0});

            m_nodes.reserve(aabbs.size() * 2);

            build(m_aabbs, 0, aabbs.size());
        }

        [[nodiscard]]
        std::vector<std::size_t> query_overlaps(const AabbType& query_aabb) const
        {
            std::vector<std::size_t> results;

            if (m_nodes.empty())
                return results;

            query_overlaps_impl(0, query_aabb, results);
            return results;
        }

        template<class RayType = Ray<>>
        [[nodiscard]]
        std::vector<HitResult> query_ray(const RayType& ray) const
        {
            std::vector<HitResult> results;

            if (m_nodes.empty())
                return results;

            query_ray_impl(0, ray, results);

            std::ranges::sort(results, [](const HitResult& a, const HitResult& b)
                              { return a.distance_sqr < b.distance_sqr; });
            return results;
        }

        [[nodiscard]]
        std::size_t node_count() const noexcept
        {
            return m_nodes.size();
        }

        [[nodiscard]]
        bool empty() const noexcept
        {
            return m_nodes.empty();
        }

    private:
        static constexpr std::size_t k_sah_bucket_count = 12;
        static constexpr std::size_t k_leaf_threshold = 1;
        static constexpr std::size_t k_null_index = std::numeric_limits<std::size_t>::max();

        struct Node
        {
            AabbType bounds;
            std::size_t left = k_null_index;
            std::size_t right = k_null_index;

            // For leaf nodes: index range into m_indices
            std::size_t first_index = 0;
            std::size_t index_count = 0;

            [[nodiscard]]
            bool is_leaf() const noexcept
            {
                return left == k_null_index;
            }
        };

        struct SahBucket
        {
            AabbType bounds = {
                {std::numeric_limits<Type>::max(), std::numeric_limits<Type>::max(),
                 std::numeric_limits<Type>::max()},
                {std::numeric_limits<Type>::lowest(), std::numeric_limits<Type>::lowest(),
                 std::numeric_limits<Type>::lowest()}
            };
            std::size_t count = 0;
        };

        [[nodiscard]]
        static constexpr Type surface_area(const AabbType& aabb) noexcept
        {
            const auto d = aabb.max - aabb.min;
            return static_cast<Type>(2) * (d.x * d.y + d.y * d.z + d.z * d.x);
        }

        [[nodiscard]]
        static constexpr AabbType merge(const AabbType& a, const AabbType& b) noexcept
        {
            return {
                {std::min(a.min.x, b.min.x), std::min(a.min.y, b.min.y), std::min(a.min.z, b.min.z)},
                {std::max(a.max.x, b.max.x), std::max(a.max.y, b.max.y), std::max(a.max.z, b.max.z)}
            };
        }

        [[nodiscard]]
        static constexpr bool overlaps(const AabbType& a, const AabbType& b) noexcept
        {
            return a.min.x <= b.max.x && a.max.x >= b.min.x
                && a.min.y <= b.max.y && a.max.y >= b.min.y
                && a.min.z <= b.max.z && a.max.z >= b.min.z;
        }

        std::size_t build(std::span<const AabbType> aabbs, std::size_t begin, std::size_t end)
        {
            const auto node_idx = m_nodes.size();
            m_nodes.emplace_back();

            auto& node = m_nodes[node_idx];
            node.bounds = compute_bounds(aabbs, begin, end);

            const auto count = end - begin;

            if (count <= k_leaf_threshold)
            {
                node.first_index = begin;
                node.index_count = count;
                return node_idx;
            }

            // Find best SAH split
            const auto centroid_bounds = compute_centroid_bounds(aabbs, begin, end);
            const auto split = find_best_split(aabbs, begin, end, node.bounds, centroid_bounds);

            // If SAH says don't split, make a leaf
            if (!split.has_value())
            {
                node.first_index = begin;
                node.index_count = count;
                return node_idx;
            }

            const auto [axis, split_pos] = split.value();

            // Partition indices around the split
            const auto mid = partition_indices(aabbs, begin, end, axis, split_pos);

            // Degenerate partition fallback: split in the middle
            const auto actual_mid = (mid == begin || mid == end) ? begin + count / 2 : mid;

            // Build children — careful: m_nodes may reallocate, so don't hold references across build calls
            const auto left_idx = build(aabbs, begin, actual_mid);
            const auto right_idx = build(aabbs, actual_mid, end);

            m_nodes[node_idx].left = left_idx;
            m_nodes[node_idx].right = right_idx;
            m_nodes[node_idx].index_count = 0;

            return node_idx;
        }

        [[nodiscard]]
        AabbType compute_bounds(std::span<const AabbType> aabbs, std::size_t begin, std::size_t end) const
        {
            AabbType bounds = {
                {std::numeric_limits<Type>::max(), std::numeric_limits<Type>::max(),
                 std::numeric_limits<Type>::max()},
                {std::numeric_limits<Type>::lowest(), std::numeric_limits<Type>::lowest(),
                 std::numeric_limits<Type>::lowest()}
            };

            for (auto i = begin; i < end; ++i)
                bounds = merge(bounds, aabbs[m_indices[i]]);

            return bounds;
        }

        [[nodiscard]]
        AabbType compute_centroid_bounds(std::span<const AabbType> aabbs, std::size_t begin, std::size_t end) const
        {
            AabbType bounds = {
                {std::numeric_limits<Type>::max(), std::numeric_limits<Type>::max(),
                 std::numeric_limits<Type>::max()},
                {std::numeric_limits<Type>::lowest(), std::numeric_limits<Type>::lowest(),
                 std::numeric_limits<Type>::lowest()}
            };

            for (auto i = begin; i < end; ++i)
            {
                const auto c = aabbs[m_indices[i]].center();
                bounds.min.x = std::min(bounds.min.x, c.x);
                bounds.min.y = std::min(bounds.min.y, c.y);
                bounds.min.z = std::min(bounds.min.z, c.z);
                bounds.max.x = std::max(bounds.max.x, c.x);
                bounds.max.y = std::max(bounds.max.y, c.y);
                bounds.max.z = std::max(bounds.max.z, c.z);
            }

            return bounds;
        }

        struct SplitResult
        {
            int axis;
            Type position;
        };

        [[nodiscard]]
        std::optional<SplitResult> find_best_split(std::span<const AabbType> aabbs, std::size_t begin,
                                                   std::size_t end, const AabbType& node_bounds,
                                                   const AabbType& centroid_bounds) const
        {
            const auto count = end - begin;
            const auto leaf_cost = static_cast<Type>(count);
            auto best_cost = leaf_cost;
            std::optional<SplitResult> best_split;

            for (int axis = 0; axis < 3; ++axis)
            {
                const auto axis_min = get_component(centroid_bounds.min, axis);
                const auto axis_max = get_component(centroid_bounds.max, axis);

                if (axis_max - axis_min < std::numeric_limits<Type>::epsilon())
                    continue;

                SahBucket buckets[k_sah_bucket_count] = {};

                const auto inv_extent = static_cast<Type>(k_sah_bucket_count) / (axis_max - axis_min);

                // Fill buckets
                for (auto i = begin; i < end; ++i)
                {
                    const auto centroid = get_component(aabbs[m_indices[i]].center(), axis);
                    auto bucket_idx = static_cast<std::size_t>((centroid - axis_min) * inv_extent);
                    bucket_idx = std::min(bucket_idx, k_sah_bucket_count - 1);

                    buckets[bucket_idx].count++;
                    if (buckets[bucket_idx].count == 1)
                        buckets[bucket_idx].bounds = aabbs[m_indices[i]];
                    else
                        buckets[bucket_idx].bounds = merge(buckets[bucket_idx].bounds, aabbs[m_indices[i]]);
                }

                // Evaluate split costs using prefix/suffix sweeps
                AabbType prefix_bounds[k_sah_bucket_count - 1];
                std::size_t prefix_count[k_sah_bucket_count - 1];

                prefix_bounds[0] = buckets[0].bounds;
                prefix_count[0] = buckets[0].count;
                for (std::size_t i = 1; i < k_sah_bucket_count - 1; ++i)
                {
                    prefix_bounds[i] = (buckets[i].count > 0)
                                           ? merge(prefix_bounds[i - 1], buckets[i].bounds)
                                           : prefix_bounds[i - 1];
                    prefix_count[i] = prefix_count[i - 1] + buckets[i].count;
                }

                AabbType suffix_bounds = buckets[k_sah_bucket_count - 1].bounds;
                std::size_t suffix_count = buckets[k_sah_bucket_count - 1].count;

                const auto parent_area = surface_area(node_bounds);
                const auto inv_parent_area = static_cast<Type>(1) / parent_area;

                for (auto i = static_cast<int>(k_sah_bucket_count) - 2; i >= 0; --i)
                {
                    const auto left_count = prefix_count[i];
                    const auto right_count = suffix_count;

                    if (left_count == 0 || right_count == 0)
                    {
                        if (i > 0)
                        {
                            suffix_bounds = (buckets[i].count > 0)
                                                ? merge(suffix_bounds, buckets[i].bounds)
                                                : suffix_bounds;
                            suffix_count += buckets[i].count;
                        }
                        continue;
                    }

                    const auto cost = static_cast<Type>(1)
                                      + (static_cast<Type>(left_count) * surface_area(prefix_bounds[i])
                                         + static_cast<Type>(right_count) * surface_area(suffix_bounds))
                                            * inv_parent_area;

                    if (cost < best_cost)
                    {
                        best_cost = cost;
                        best_split = SplitResult{
                            axis,
                            axis_min + static_cast<Type>(i + 1) * (axis_max - axis_min)
                                           / static_cast<Type>(k_sah_bucket_count)
                        };
                    }

                    suffix_bounds = (buckets[i].count > 0)
                                        ? merge(suffix_bounds, buckets[i].bounds)
                                        : suffix_bounds;
                    suffix_count += buckets[i].count;
                }
            }

            return best_split;
        }

        std::size_t partition_indices(std::span<const AabbType> aabbs, std::size_t begin, std::size_t end,
                                      int axis, Type split_pos)
        {
            auto it = std::partition(m_indices.begin() + static_cast<std::ptrdiff_t>(begin),
                                     m_indices.begin() + static_cast<std::ptrdiff_t>(end),
                                     [&](std::size_t idx)
                                     { return get_component(aabbs[idx].center(), axis) < split_pos; });

            return static_cast<std::size_t>(std::distance(m_indices.begin(), it));
        }

        [[nodiscard]]
        static constexpr Type get_component(const Vector3<Type>& v, int axis) noexcept
        {
            switch (axis)
            {
            case 0:
                return v.x;
            case 1:
                return v.y;
            default:
                return v.z;
            }
        }

        void query_overlaps_impl(std::size_t node_idx, const AabbType& query_aabb,
                                 std::vector<std::size_t>& results) const
        {
            const auto& node = m_nodes[node_idx];

            if (!overlaps(node.bounds, query_aabb))
                return;

            if (node.is_leaf())
            {
                for (auto i = node.first_index; i < node.first_index + node.index_count; ++i)
                    if (overlaps(query_aabb, m_aabbs[m_indices[i]]))
                        results.push_back(m_indices[i]);
                return;
            }

            query_overlaps_impl(node.left, query_aabb, results);
            query_overlaps_impl(node.right, query_aabb, results);
        }

        template<class RayType>
        void query_ray_impl(std::size_t node_idx, const RayType& ray,
                            std::vector<HitResult>& results) const
        {
            const auto& node = m_nodes[node_idx];

            // Quick AABB-ray rejection using the slab method
            const auto hit = LineTracer<RayType>::get_ray_hit_point(ray, node.bounds);
            if (hit == ray.end)
                return;

            if (node.is_leaf())
            {
                for (auto i = node.first_index; i < node.first_index + node.index_count; ++i)
                {
                    const auto leaf_hit = LineTracer<RayType>::get_ray_hit_point(
                        ray, m_aabbs[m_indices[i]]);
                    if (leaf_hit != ray.end)
                    {
                        const auto diff = leaf_hit - ray.start;
                        results.push_back({m_indices[i], diff.dot(diff)});
                    }
                }
                return;
            }

            query_ray_impl(node.left, ray, results);
            query_ray_impl(node.right, ray, results);
        }

        std::vector<Node> m_nodes;
        std::vector<std::size_t> m_indices;
        std::vector<AabbType> m_aabbs;
    };
} // namespace omath::collision
