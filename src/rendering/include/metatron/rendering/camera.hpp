#include <metatron/rendering/ray.hpp>
#include <metatron/core/math/vector.hpp>

namespace metatron::rendering {
	struct Camera final {
		auto sample_ray(math::Vector<f32, 2> const& p) -> Ray;
	};
}
