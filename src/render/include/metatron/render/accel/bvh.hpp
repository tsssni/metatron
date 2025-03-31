#include <metatron/render/accel/accel.hpp>
#include <vector>

namespace metatron::accel {
	struct LBVH final: Acceleration {
		LBVH(std::vector<Divider>&& dividers);
		auto operator()(
			math::Ray const& r
		) const -> std::optional<Divider const*>;

	private:
		std::vector<Divider> dividers;
	};
}
