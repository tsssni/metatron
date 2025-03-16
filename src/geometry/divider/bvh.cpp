#include <metatron/geometry/divider/bvh.hpp>

namespace metatron::divider {
	LBVH::LBVH(std::vector<Divider const*>&& dividers)
		: dividers(std::move(dividers)) {}

	auto LBVH::operator()(math::Ray const& r) -> std::optional<Divider const*> {
		return dividers.front();
	}
}
