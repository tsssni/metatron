#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>

namespace metatron::photo {
	struct Lens {
		virtual ~Lens() {}
		auto virtual sample(math::Vector<f32, 3> o, math::Vector<f32, 2> u) -> math::Ray = 0;
	};
}
