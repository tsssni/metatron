#include <metatron/render/divider/bvh.hpp>

namespace metatron::divider {
	LBVH::LBVH(std::vector<Divider const*>&& dividers)
		: dividers(std::move(dividers)) {}

	auto LBVH::intersect(math::Ray const& r) -> std::optional<Intersection> {
		auto intr = dividers[0]->shape->intersect(r);
		if (!intr) return {};
		else return Intersection{dividers[0], intr.value()};
	}
}
