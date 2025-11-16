#include <metatron/render/accel/lbvh.hpp>
#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/core/math/encode.hpp>
#include <metatron/core/stl/thread.hpp>
#include <ranges>
#include <stack>

namespace mtt::accel {
    LBVH::LBVH(cref<Descriptor> desc) noexcept {
        struct Node final {
            math::Bounding_Box bbox;
            obj<Node> left;
            obj<Node> right;
            u32 morton_code;
            u32 split_axis;
            u32 div_idx;
            u32 num_prims{0u};
        };

        auto& divs = stl::vector<Divider>::instance();
        auto prims = std::vector<Primitive>{};
        auto bvh = std::vector<Index>{};
        for (auto i = 0u; i < divs.size(); ++i) {
            auto div = divs[i];
            auto s = div->shape;
            for (auto j = 0u; j < s->size(); j++) {
                auto lt = div->local_to_render;
                prims.push_back(Primitive{
                    .bbox = s->bounding_box(lt->transform, j),
                    .instance = i,
                    .primitive = j,
                });
            }
        }

        auto render_bbox = math::Bounding_Box{};
        for (auto& p: prims)
            render_bbox = math::merge(render_bbox, p.bbox);
        for (auto& p: prims) {
            auto extent = render_bbox.p_max - render_bbox.p_min;
            auto pos = math::lerp(p.bbox.p_min, p.bbox.p_max, 0.5f) - render_bbox.p_min;
            auto voxel = uv3{pos / extent * 1024};
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

        auto morton_split = [&](this auto self, uv2 interval, i32 bit) -> obj<Node> {
            auto [start, end] = interval;
            auto n = end - start;
            if (bit < 0 || n <= desc.num_guide_leaf_prims) {
                auto node = make_obj<Node>();
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

                auto node = make_obj<Node>();
                node->left = self({start, split}, bit - 1);
                node->right = self({split, end}, bit - 1);
                node->bbox = math::merge(node->left->bbox, node->right->bbox);
                node->split_axis = bit % 3;
                return node;
            }
        };
        auto lbvh_nodes = std::vector<obj<Node>>(intervals.size());
        stl::scheduler::instance().sync_parallel(
            uzv1{intervals.size()},
            [&](auto idx) {
                auto [i] = idx;
                auto& interval = intervals[i];
                lbvh_nodes[i] = morton_split(interval, 29 - 12);
            }
        );

        auto area_split = [&](this auto self, rref<std::vector<obj<Node>>> nodes) -> obj<Node> {
            if (nodes.size() == 0) return nullptr;
            else if (nodes.size() == 1) return std::move(nodes.front());

            auto root = make_obj<Node>();
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
                auto s = math::foreach([](auto& bbox, usize i) {
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
                    | std::ranges::to<std::vector<obj<Node>>>();
            };
            auto left = range_split(std::ranges::begin(nodes), std::ranges::begin(splitted_iter));
            auto right = range_split(std::ranges::begin(splitted_iter), std::ranges::end(nodes));
            root->left = self(std::move(left));
            root->right = self(std::move(right));
            return root;
        };
        auto root = area_split(std::move(lbvh_nodes));

        // pre-order binary tree traversal
        auto traverse = [&bvh](this auto self, view<Node> node) -> void {
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

        auto lock = stl::arena::instance().lock();
        this->prims = std::span{prims};
        this->bvh = std::span{bvh};
    }

    auto LBVH::operator()(
        cref<math::Ray> r, cref<fv3> n
    ) const noexcept -> opt<Interaction> {
        using Storage = std::vector<u32>;
        auto prim = view<Primitive>{};
        auto q_opt = opt<f32>{};
        auto storage = std::vector<u32>{};
        auto stack = std::stack<u32, decltype(storage)>{};
        storage.reserve(128);
        stack = decltype(stack){std::move(storage)};
        stack.push(0u);

        while(!stack.empty()) {
            auto idx = stack.top();
            stack.pop();
            auto node = &bvh[idx];
            auto b_opt = math::hit(r, node->bbox);
            if (!b_opt || (q_opt && *q_opt < b_opt.value()[0])) continue;

            if (node->num_prims < 0) {
                for (auto i = 0u; i < -node->num_prims; ++i) {
                    auto idx = node->prim + i;
                    auto& p = prims[idx];
                    auto div = p.instance;
                    auto lr = (*p.instance->local_to_render) ^ r;

                    MTT_OPT_OR_CONTINUE(t, div->shape->query(lr, p.primitive));
                    if (!q_opt || t < *q_opt) {
                        q_opt = t_opt;
                        prim = &p;
                    }
                }
            } else {
                if (r.d[node->axis] < 0.f) {
                    stack.push(idx + 1);
                    stack.push(node->right);
                } else {
                    stack.push(node->right);
                    stack.push(idx + 1);
                }
            }
        }

        return !prim ? opt<Interaction>{} : Interaction{
            .divider = prim->instance,
            .primitive = prim->primitive,
            .intr_opt = (*prim->instance->shape.data())(
                (*prim->instance->local_to_render) ^ r,
                (*prim->instance->local_to_render) ^ n,
                prim->primitive
            ),
        };
    }
}
