#pragma once
#include <metatron/core/math/vector.hpp>

namespace metatron::math {
	struct Sampler {
		auto virtual start(math::Vector<usize, 2> const& pixel, usize idx) -> void = 0;
		auto virtual generate_1d() const -> f32 = 0;
		auto virtual generate_2d() const -> math::Vector<f32, 2> = 0;
		auto virtual generate_pixel_2d() const -> math::Vector<f32, 2> = 0;
	};
}
