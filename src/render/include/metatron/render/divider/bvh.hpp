#include <metatron/render/divider/accel.hpp>
#include <vector>

namespace metatron::divider {
	struct LBVH final: Acceleration {
		LBVH(std::vector<Divider const*>&& dividers);
		auto operator()(math::Ray const& r) -> std::optional<Interaction>;

	private:
		std::vector<Divider const*> dividers;
	};
}
