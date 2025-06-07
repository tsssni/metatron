#include <metatron/render/accel/accel.hpp>
#include <vector>

namespace metatron::accel {
	struct LBVH final: Acceleration {
		struct alignas(32) Node final {
			math::Bounding_Box bbox;
			union {
				u32 div_idx;
				u32 r_idx;
			};
			u16 num_prims;
			byte axis;
		};

		LBVH(
			std::vector<Divider>&& dividers,
			math::Transform const* transform,
			usize num_guide_leaf_prims = 4
		);
		auto operator()(
			math::Ray const& r,
			math::Vector<f32, 3> const& n = {}
		) const -> std::optional<Interaction>;

	private:
		std::vector<Node> bvh;
		std::vector<Divider> dividers;
		math::Transform const* transform;
	};
}
