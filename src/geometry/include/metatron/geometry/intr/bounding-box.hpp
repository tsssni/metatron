#pragma once
#include <metatron/core/math/vector.hpp>

namespace metatron::intr {
	struct Bounding_Box final {
		math::Vector<f32, 3> p_min;
		math::Vector<f32, 3> p_max;
	};
}
