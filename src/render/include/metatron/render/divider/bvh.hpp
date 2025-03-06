#include <metatron/render/divider/accel.hpp>
#include <vector>

namespace metatron::divider {
	struct LBVH final: Acceleration {
		LBVH(std::vector<Divider const*>&& dividers);
		auto intersect(math::Ray const& r) -> std::optional<Intersection>;

	private:
		std::vector<Divider const*> dividers;
	};
}
