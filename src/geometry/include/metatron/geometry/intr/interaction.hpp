#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>

namespace metatron::intr {
	struct Interaction final {
		math::Vector<f32, 3> p;
		math::Vector<f32, 3> n;
		math::Vector<f32, 2> uv;
		f32 pdf;
	};

	struct Context final {
		math::Vector<f32, 3> p;
		math::Vector<f32, 3> n;
		math::Ray r;
	};
}
