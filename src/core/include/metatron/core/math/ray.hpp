#pragma once
#include <metatron/core/math/vector.hpp>

namespace mtt::math {
	struct Ray final {
		math::Vector<f32, 3> o;
		math::Vector<f32, 3> d;
	};

	struct Ray_Differential final {
		Ray r;
		Ray rx;
		Ray ry;
		bool differentiable;
	};
}
