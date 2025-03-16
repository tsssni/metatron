#include <metatron/geometry/divider/bvh.hpp>

namespace metatron::divider {
	LBVH::LBVH(std::vector<Divider const*>&& dividers)
		: dividers(std::move(dividers)) {}

	auto LBVH::operator()(math::Ray const& r) -> std::optional<Interaction> {
		auto intr = (*dividers[0]->shape)(r);
		if (!intr) return {};
		else return Interaction{dividers[0], intr.value()};
	}
}
