#include <metatron/render/divider/bvh.hpp>

namespace metatron::divider {
	LBVH::LBVH(std::vector<Divider const*>&& dividers)
		: dividers(std::move(dividers)) {}

	auto LBVH::operator()(math::Ray const& r) const -> std::optional<Divider const*> {
		// TODO: just test
		return dividers.front();
	}
}
