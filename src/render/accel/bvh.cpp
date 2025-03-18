#include <metatron/render/accel/bvh.hpp>

namespace metatron::accel {
	LBVH::LBVH(std::vector<Divider>&& dividers)
		: dividers(std::move(dividers)) {}

	auto LBVH::operator()(math::Ray const& r) const -> std::optional<Divider const*> {
		// TODO: just test
		return &dividers.front();
	}
}
