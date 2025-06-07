#include <metatron/render/accel/accel.hpp>
#include <vector>

namespace metatron::accel {
	struct LBVH final: Acceleration {
		LBVH(
			std::vector<Divider>&& dividers,
			math::Transform const* world_to_render
		);
		auto operator()(
			math::Ray const& r,
			math::Vector<f32, 3> const& np = {}
		) const -> std::optional<Interaction>;

	private:
		std::vector<Divider> dividers;
		math::Transform const* world_to_render;
	};
}
