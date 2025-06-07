#include <metatron/render/accel/bvh.hpp>

namespace metatron::accel {
	LBVH::LBVH(
		std::vector<Divider>&& dividers,
		math::Transform const* world_to_render
	):
	dividers(std::move(dividers)),
	world_to_render(world_to_render) {}

	auto LBVH::operator()(
		math::Ray const& r,
		math::Vector<f32, 3> const& np
	) const -> std::optional<Interaction> {
		auto div = &dividers.front();
		auto lr = *div->local_to_world ^ *world_to_render ^ r;
		return Interaction{div, (*div->shape)(lr, np, div->primitive)};
	}
}
