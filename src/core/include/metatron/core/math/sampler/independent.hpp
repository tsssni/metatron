#pragma once
#include <metatron/core/math/sampler/sampler.hpp>
#include <random>

namespace metatron::math {
	struct Independent_Sampler final: Sampler {
		Independent_Sampler(usize seed = 0);

		auto start(math::Vector<usize, 2> const& pixel, usize idx) -> void;
		auto generate_1d() const -> f32;
		auto generate_2d() const -> math::Vector<f32, 2>;
		auto generate_pixel_2d() const -> math::Vector<f32, 2>;
	
	private:
		mutable std::mt19937 rng;
		mutable std::uniform_real_distribution<f32> dist;
		math::Vector<usize, 2> pixel;
		usize idx;
	};
}
