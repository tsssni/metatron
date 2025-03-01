#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>

namespace metatron::render {
	struct Camera final {
		auto sample(math::Vector<f32, 2> const& p);
	};
}
