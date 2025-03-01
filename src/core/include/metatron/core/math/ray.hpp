#pragma once
#include <metatron/core/math/vector.hpp>

namespace metatron::math {
	struct Ray final {
		math::Vector<f32, 3> o;
		math::Vector<f32, 3> d;
	};
}
