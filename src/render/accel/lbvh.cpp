#include <metatron/render/accel/lbvh.hpp>
#include <metatron/core/math/encode.hpp>
#include <metatron/core/stl/optional.hpp>
#include <ranges>
#include <stack>

namespace metatron::accel {
	struct LBVH_Divider final {
		math::Bounding_Box bbox;
		u32 index;
		u32 morton_code;
	};

	struct LBVH_Node final {
		math::Bounding_Box bbox;
		std::unique_ptr<LBVH_Node> left;
		std::unique_ptr<LBVH_Node> right;
		u32 morton_code;
		u32 split_axis;
		u32 div_idx;
		u32 num_prims{0u};
	};

	LBVH::LBVH(
		std::vector<Divider>&& dividers,
		math::Transform const* transform,
		usize num_guide_left_prims
	):
	dividers(std::move(dividers)),
	transform(transform) {
		std::vector<LBVH_Divider> lbvh_divs;
		for (auto i = 0u; i < this->dividers.size(); i++) {
			auto& div = this->dividers[i];
			auto& lt = *div.local_to_world;
			auto t = this->transform->transform | lt.transform;
			lbvh_divs.push_back(LBVH_Divider{
				.bbox = div.shape->bounding_box(&t, div.primitive),
				.index = i,
			});
		}

		auto render_bbox = math::Bounding_Box{};
		for (auto& div: lbvh_divs) {
			render_bbox = math::merge(render_bbox, div.bbox);
		}
		for (auto& div: lbvh_divs) {
			auto extent = render_bbox.p_max - render_bbox.p_min;
			auto pos = math::lerp(div.bbox.p_min, div.bbox.p_max, 0.5f) - render_bbox.p_min;
			auto voxel = math::Vector<u32, 3>{math::guarded_div(pos, extent) * 1024};
			div.morton_code = math::morton_encode(voxel);
		}
		std::ranges::sort(lbvh_divs, [](LBVH_Divider const& a, LBVH_Divider const& b) {
			return a.morton_code < b.morton_code;
		});

		auto intervals = std::vector<math::Vector<u32, 2>>{};
		for (auto start = 0u, end = 0u; end <= lbvh_divs.size(); end++) {
			auto constexpr mask = 0x3ffc0000;
			if (false
			|| end == lbvh_divs.size()
			|| (lbvh_divs[start].morton_code & mask) != (lbvh_divs[end].morton_code & mask)) {
				intervals.push_back({start, end});
				start = end;
			}
		}

		auto morton_split = [&](this auto self, math::Vector<u32, 2> interval, i32 bit) -> std::unique_ptr<LBVH_Node> {
			auto [start, end] = interval;
			auto n = end - start;
			if (n <= num_guide_left_prims || bit < 0) {
				auto node = std::make_unique<LBVH_Node>();
				node->div_idx = start;
				node->num_prims = n;
				node->bbox = math::Bounding_Box{};
				for (auto i = start; i < start + n; i++) {
					node->bbox = math::merge(node->bbox, lbvh_divs[i].bbox);
				}
				return node;
			} else {
				auto mask = 1u << bit;
				auto start_split_bit = lbvh_divs[start].morton_code & mask;
				auto split = start + 1;
				for (; split < end; split++) {
					auto split_bit = lbvh_divs[split].morton_code & mask;
					if (split_bit != start_split_bit) {
						break;
					}
				}

				if (split == end) {
					return self(interval, bit - 1);
				}

				auto node = std::make_unique<LBVH_Node>();
				node->left = self({start, split}, bit - 1);
				node->right = self({split, end}, bit - 1);
				node->bbox = math::merge(node->left->bbox, node->right->bbox);
				node->split_axis = bit % 3;
				return node;
			}
		};
		auto lbvh_nodes = std::vector<std::unique_ptr<LBVH_Node>>(intervals.size());
		for (auto i = 0u; i < intervals.size(); i++) {
			auto& interval = intervals[i];
			lbvh_nodes[i] = morton_split(interval, 29 - 12);
		}

		auto area_split = [&](this auto self, std::vector<std::unique_ptr<LBVH_Node>>&& nodes) -> std::unique_ptr<LBVH_Node> {
			if (nodes.size() == 1) {
				return std::move(nodes.front());
			}

			auto root = std::make_unique<LBVH_Node>();
			root->bbox = math::Bounding_Box{};
			for (auto& node: nodes) {
				root->bbox = math::merge(root->bbox, node->bbox);
			}
			
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
				auto b = std::min(num_buckets - 1, i32(num_buckets
					* (c[root->split_axis] - cbox.p_min[root->split_axis])
					/ (cbox.p_max[root->split_axis] - cbox.p_min[root->split_axis]))
				);
				auto& [bbox, count] = buckets[b];
				bbox = math::merge(bbox, node->bbox);
				count++;
			}

			auto sah = std::vector<f32>(num_buckets - 1);
			for (auto i = 0; i < num_buckets - 1; i++) {
				auto b = math::Vector<math::Bounding_Box, 2>{};
				auto c = math::Vector<i32, 2>{};
				for (auto j = 0; j <= i; j++) {
					auto& [bbox, count] = buckets[j];
					b[0] = math::merge(b[0], bbox);
					c[0] += count;
				}
				for (auto j = i + 1; j < num_buckets; j++) {
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
				auto b = std::min(num_buckets - 1, i32(num_buckets
					* (c[root->split_axis] - cbox.p_min[root->split_axis])
					/ (cbox.p_max[root->split_axis] - cbox.p_min[root->split_axis]))
				);
				return b < split_idx;
			});

			auto range_split = [](auto&& begin, auto&& end){
				return std::ranges::subrange(begin, end)
					 | std::views::transform([](auto& n) { return std::move(n); })
					 | std::ranges::to<std::vector<std::unique_ptr<LBVH_Node>>>();
			};
			auto left = range_split(std::ranges::begin(nodes), std::ranges::begin(splitted_iter));
			auto right = range_split(std::ranges::begin(splitted_iter), std::ranges::end(nodes));
			root->left = self(std::move(left));
			root->right = self(std::move(right));
			return root;
		};
		auto root = area_split(std::move(lbvh_nodes));

		// pre-order binary tree traversal
		auto traverse = [&bvh = this->bvh](this auto self, LBVH_Node const* node) -> void {
			if (node->num_prims > 0) {
				bvh.push_back({
					.bbox = node->bbox,
					.div_idx = node->div_idx,
					.num_prims = u16(node->num_prims),
					.axis = byte(node->split_axis),
				});
			} else {
				bvh.push_back({
					.bbox = node->bbox,
					.num_prims = 0u,
					.axis = byte(node->split_axis),
				});
				auto idx = bvh.size() - 1;
				self(node->left.get());
				bvh[idx].r_idx = u32(bvh.size());
				self(node->right.get());
			}
		};
		traverse(root.get());

		auto divs = std::vector<Divider>(lbvh_divs.size());
		for (auto i = 0u; i < lbvh_divs.size(); i++) {
			divs[i] = this->dividers[lbvh_divs[i].index];
		}
		std::swap(divs, this->dividers);
	}

	auto LBVH::operator()(
		math::Ray const& r,
		math::Vector<f32, 3> const& n
	) const -> std::optional<Interaction> {
		auto& rt = *transform;
		auto intr_div = (Divider const*)nullptr;
		auto intr_opt = std::optional<shape::Interaction>{};
		auto candidates = std::stack<u32>{};
		candidates.push(0u);

		while(!candidates.empty()) {
			auto idx = candidates.top();
			candidates.pop();
			auto node = &bvh[idx];
			METATRON_OPT_OR_CONTINUE(t_bbox, math::hit(r, node->bbox));
			if (intr_opt && t_bbox >= intr_opt.value().t) {
				continue;
			}

			if (node->num_prims > 0) {
				for (auto i = 0u; i < node->num_prims; i++) {
					auto idx = node->div_idx + i;
					auto div = &dividers[idx];
					auto& lt = *div->local_to_world;

					auto lr = lt ^ rt ^ r;
					auto ln = lt ^ rt ^ n;
					METATRON_OPT_OR_CONTINUE(div_intr, (*div->shape)(lr, ln, div->primitive));

					if (!intr_opt || div_intr.t < intr_opt.value().t) {
						intr_opt = div_intr_opt;
						intr_div = div;
					}
				}
			} else {
				if (r.d[node->axis] < 0.f) {
					candidates.push(idx + 1);
					candidates.push(node->r_idx);
				} else {
					candidates.push(node->r_idx);
					candidates.push(idx + 1);
				}
			}
		}

		return Interaction{intr_div, intr_opt};
	}
}
