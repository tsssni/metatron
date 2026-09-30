#include <metatron/render/accel/lbvh.hpp>
#include <metatron/core/math/encode.hpp>
#include <metatron/core/stl/thread.hpp>

namespace mtt::accel {
    auto LBVH::build(
        std::vector<math::Bounding_Box> const& boxes,
        u32 num_guide_leaf_prims
    ) noexcept -> std::tuple<std::vector<u32>, std::vector<Index>> {
        struct Node final {
            math::Bounding_Box bbox;
            std::unique_ptr<Node> left;
            std::unique_ptr<Node> right;
            u32 morton_code;
            u32 split_axis;
            u32 div_idx;
            u32 num_prims{0u};
        };

        struct Primitive final {
            math::Bounding_Box bbox;
            u32 idx;
            u32 morton_code;
        };

        if (boxes.empty()) return {};
        auto prims = std::vector<Primitive>(boxes.size());
        auto bvh = std::vector<Index>{};
        for (auto i = 0u; i < boxes.size(); ++i)
            prims[i] = {.bbox = boxes[i], .idx = i};

        auto render_bbox = math::Bounding_Box{};
        for (auto& p: prims)
            render_bbox = math::merge(render_bbox, p.bbox);
        for (auto& p: prims) {
            auto extent = math::max(render_bbox.p_max - render_bbox.p_min, fv3{math::epsilon<f32>});
            auto pos = math::lerp(p.bbox.p_min, p.bbox.p_max, 0.5f) - render_bbox.p_min;
            auto voxel = uv3{math::min(pos / extent * 1024.f, fv3{1023.f})};
            p.morton_code = math::morton_encode(voxel);
        }
        std::ranges::sort(prims, [](auto& a, auto& b) {
            return a.morton_code < b.morton_code;
        });

        auto intervals = std::vector<uv2>{};
        for (auto start = 0u, end = 0u; end <= prims.size(); ++end) {
            auto constexpr mask = 0x3ffc0000;
            if (false
            || end == prims.size()
            || (prims[start].morton_code & mask) != (prims[end].morton_code & mask)) {
                intervals.push_back({start, end});
                start = end;
            }
        }

        auto morton_split = [&](this auto self, uv2 interval, i32 bit) -> std::unique_ptr<Node> {
            auto [start, end] = interval;
            auto n = end - start;
            if (bit < 0 || n <= num_guide_leaf_prims) {
                auto node = std::make_unique<Node>();
                node->div_idx = start;
                node->num_prims = n;
                node->bbox = math::Bounding_Box{};
                for (auto i = start; i < end; ++i)
                    node->bbox = math::merge(node->bbox, prims[i].bbox);
                return node;
            } else {
                auto mask = 1u << bit;
                auto start_split_bit = prims[start].morton_code & mask;
                auto split = start + 1;
                for (; split < end; ++split) {
                    auto split_bit = prims[split].morton_code & mask;
                    if (split_bit != start_split_bit) break;
                }
                if (split == end) return self(interval, bit - 1);

                auto node = std::make_unique<Node>();
                node->left = self({start, split}, bit - 1);
                node->right = self({split, end}, bit - 1);
                node->bbox = math::merge(node->left->bbox, node->right->bbox);
                node->split_axis = bit % 3;
                return node;
            }
        };
        auto lbvh_nodes = std::vector<std::unique_ptr<Node>>(intervals.size());
        stl::scheduler::sync_parallel(
            uzv1{intervals.size()},
            [&](auto idx) {
                auto [i] = idx;
                auto& interval = intervals[i];
                lbvh_nodes[i] = morton_split(interval, 29 - 12);
            }
        );

        auto area_split = [&](this auto self, std::vector<std::unique_ptr<Node>>&& nodes) -> std::unique_ptr<Node> {
            if (nodes.size() == 0) return nullptr;
            else if (nodes.size() == 1) return std::move(nodes.front());

            auto root = std::make_unique<Node>();
            root->bbox = math::Bounding_Box{};
            for (auto& node: nodes)
                root->bbox = math::merge(root->bbox, node->bbox);

            auto cbox = math::Bounding_Box{};
            for (auto& node: nodes) {
                auto c = math::lerp(node->bbox.p_min, node->bbox.p_max, 0.5f);
                cbox = math::merge(cbox, {c, c});
            }
            root->split_axis = math::maxi(math::abs(cbox.p_max - cbox.p_min));

            auto constexpr num_buckets = 12;
            auto buckets = std::vector<std::tuple<math::Bounding_Box, i32>>(num_buckets);
            for (auto& node: nodes) {
                auto c = math::lerp(node->bbox.p_min, node->bbox.p_max, 0.5f);
                auto b = math::min(num_buckets - 1, i32(num_buckets
                * math::guarded_div(
                    c[root->split_axis] - cbox.p_min[root->split_axis],
                    cbox.p_max[root->split_axis] - cbox.p_min[root->split_axis]
                )));
                auto& [bbox, count] = buckets[b];
                bbox = math::merge(bbox, node->bbox);
                ++count;
            }

            auto sah = std::vector<f32>(num_buckets - 1);
            for (auto i = 0; i < num_buckets - 1; ++i) {
                auto b = math::Vector<math::Bounding_Box, 2>{};
                auto c = iv2{};
                for (auto j = 0; j <= i; ++j) {
                    auto& [bbox, count] = buckets[j];
                    b[0] = math::merge(b[0], bbox);
                    c[0] += count;
                }
                for (auto j = i + 1; j < num_buckets; ++j) {
                    auto& [bbox, count] = buckets[j];
                    b[1] = math::merge(b[1], bbox);
                    c[1] += count;
                }
                auto s = math::foreach([](auto& bbox, auto) {
                    return math::area(bbox);
                }, b);
                sah[i] = 0.125f + math::sum(math::mul(s, c)) / math::area(root->bbox);
            }

            auto split_idx = std::ranges::distance(sah.begin(), std::ranges::min_element(sah)) + 1;
            auto splitted_iter = std::ranges::partition(nodes, [&](auto& node) {
                auto c = math::lerp(node->bbox.p_min, node->bbox.p_max, 0.5f);
                auto b = math::min(num_buckets - 1, i32(num_buckets
                * math::guarded_div(
                    c[root->split_axis] - cbox.p_min[root->split_axis],
                    cbox.p_max[root->split_axis] - cbox.p_min[root->split_axis]
                )));
                return b < split_idx;
            });

            if (false
            || std::ranges::begin(splitted_iter) == std::ranges::begin(nodes)
            || std::ranges::begin(splitted_iter) == std::ranges::end(nodes)) {
                // avoid stack overflow by all splitted to one child
                splitted_iter = std::ranges::subrange(std::ranges::begin(nodes) + 1, std::ranges::end(nodes));
            }

            auto range_split = [](auto&& begin, auto&& end){
                return std::ranges::subrange(begin, end)
                | std::views::transform([](auto& n) { return std::move(n); })
                | std::ranges::to<std::vector<std::unique_ptr<Node>>>();
            };
            auto left = range_split(std::ranges::begin(nodes), std::ranges::begin(splitted_iter));
            auto right = range_split(std::ranges::begin(splitted_iter), std::ranges::end(nodes));
            root->left = self(std::move(left));
            root->right = self(std::move(right));
            return root;
        };
        auto root = area_split(std::move(lbvh_nodes));

        // pre-order binary tree traversal
        auto traverse = [&bvh](this auto self, Node const* node) -> void {
            if (node->num_prims > 0) {
                bvh.push_back({
                    .bbox = node->bbox,
                    .prim = node->div_idx,
                    .num_prims = -i32(node->num_prims),
                });
            } else {
                bvh.push_back({
                    .bbox = node->bbox,
                    .axis = node->split_axis,
                });
                auto idx = bvh.size() - 1;
                self(node->left.get()); bvh[idx].right = u32(bvh.size()); self(node->right.get());
            }
        };
        traverse(root.get());

        auto order = prims
        | std::views::transform([](auto& p) { return p.idx; })
        | std::ranges::to<std::vector<u32>>();
        return {std::move(order), std::move(bvh)};
    }

    LBVH::LBVH(Descriptor const& desc) noexcept {
        using shapes = shape::Shape::vs;
        auto num_meshes = shapes::size<shape::Mesh>();
        blas = buf<buf<Index>>(num_meshes);
        prims = buf<buf<u32>>(num_meshes);
        for (auto i = 0u; i < num_meshes; ++i) {
            auto& mesh = *shapes::get<shape::Mesh>(i);
            auto boxes = std::vector<math::Bounding_Box>(mesh.size());
            for (auto j = 0u; j < mesh.size(); ++j)
                boxes[j] = mesh.bounding_box(math::Transform{}, j);
            auto [order, nodes] = build(boxes, desc.num_guide_leaf_prims);
            blas[i] = buf<Index>{std::span{nodes}};
            prims[i] = buf<u32>{std::span{order}};
        }

        using divs = stl::vector<Divider>;
        auto candidates = std::vector<u32>{};
        auto boxes = std::vector<math::Bounding_Box>{};
        for (auto i = 0u; i < divs::size(); ++i) {
            auto& div = *divs::get(i);
            auto s = div.shape;
            auto bbox = math::Bounding_Box{};
            if (s.is<shape::Mesh>()) {
                auto& nodes = blas[s.idx.index()];
                if (nodes.empty()) continue;
                bbox = *div.local_to_render | nodes[0].bbox;
            } else {
                if (s.size() == 0) continue;
                for (auto j = 0u; j < s.size(); ++j)
                    bbox = math::merge(bbox, s.bounding_box(div.local_to_render, j));
            }
            candidates.push_back(i);
            boxes.push_back(bbox);
        }

        auto [order, nodes] = build(boxes, desc.num_guide_leaf_prims);
        auto instances = order
        | std::views::transform([&](auto i) { return candidates[i]; })
        | std::ranges::to<std::vector<u32>>();

        this->instances = std::span{instances};
        this->tlas = std::span{nodes};
    }

    auto LBVH::traverse(
        proxy::Divider div, Interaction& intr,
        math::Ray const& r, Flags flags, fv2 const& range
    ) const noexcept -> bool {
        auto k = div ? div->shape.idx.index() : 0u;
        auto& bvh = div ? blas[k] : tlas;

        auto query = [&](math::Ray const& r, proxy::Divider d, u32 j) -> bool {
            auto t = d->shape.query(r, j);
            if (t[3] < range[0] || t[3] >= intr.pos[3]) return false;
            intr = {.divider = d, .primitive = j, .pos = t};
            return flags & Flags::hit_first;
        };

        auto inv_d = 1.f / r.d;
        auto stack = std::array<u32, 64>{};
        auto top = 0uz;
        stack[top++] = 0u;

        while (top > 0) {
            auto idx = stack[--top];
            auto node = &bvh[idx];
            auto b = math::hit(r, inv_d, node->bbox);
            if (false
            || b[1] < range[0] - math::epsilon<f32>
            || b[0] > b[1] + math::epsilon<f32>
            || intr.pos[3] < b[0]
            ) continue;

            if (node->num_prims >= 0) {
                if (r.d[node->axis] < 0.f) {
                    stack[top++] = idx + 1;
                    stack[top++] = node->right;
                } else {
                    stack[top++] = node->right;
                    stack[top++] = idx + 1;
                }
                continue;
            }

            for (auto i = node->prim; i < node->prim + u32(-node->num_prims); ++i) {
                if (div) {
                    if (query(r, div, prims[k][i])) return true;
                    continue;
                }

                auto d = proxy::Divider{instances[i]};
                auto is_interface = d->material && d->material.is<material::Interface_Material>();
                if ((flags & Flags::skip_interface) && is_interface) continue;
                if ((flags & Flags::only_interface) && !is_interface) continue;

                auto lr = d->local_to_render ^ r;
                if (d->shape.is<shape::Mesh>()) {
                    if (traverse(d, intr, lr, flags, range)) return true;
                } else {
                    for (auto j = 0u; j < d->shape.size(); ++j)
                        if (query(lr, d, j)) return true;
                }
            }
        }
        return false;
    }

    auto LBVH::operator()(
        math::Ray const& r, fv3 const& n,
        Flags flags, fv2 const& range
    ) const noexcept -> Interaction {
        if (tlas.empty()) return {};

        auto intr = Interaction{};
        intr.pos[3] = range[1];
        traverse({}, intr, r, flags, range);
        if (!intr.divider) return {};
        return intr;
    }
}
