#include <metatron/render/ray.hpp>
#include <metatron/core/math/vector.hpp>

namespace metatron::render {
	struct Camera final {
		auto sample_ray(math::Vector<f32, 2> const& p) -> Ray;
	};
}
