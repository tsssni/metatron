#pragma once
#include <metatron/core/math/sampler/sampler.hpp>
#include <random>

namespace mtt::math {
	struct Independent_Sampler final {
		Independent_Sampler(usize seed = 0) noexcept;
		Independent_Sampler(Independent_Sampler const&) noexcept = default;

		auto start(Vector<usize, 2> const& pixel, usize idx, usize dim = 0uz) noexcept -> void;
		auto generate_1d() const noexcept -> f32;
		auto generate_2d() const noexcept -> Vector<f32, 2>;
		auto generate_pixel_2d() const noexcept -> Vector<f32, 2>;
	
	private:
		mutable std::mt19937 rng;
		mutable std::uniform_real_distribution<f32> dist;
		Vector<usize, 2> pixel;
		usize idx;
		usize dim;
	};
}
