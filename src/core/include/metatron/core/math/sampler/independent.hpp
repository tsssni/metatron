#pragma once
#include <metatron/core/math/sampler/sampler.hpp>
#include <random>

namespace mtt::math {
	struct Independent_Sampler final {
		Independent_Sampler(usize seed = 0) noexcept:
		rng(seed), dist(1e-4, 1.f - 1e-4) {}
		Independent_Sampler(Independent_Sampler const&) noexcept = default;

		auto start(Vector<usize, 2> const& pixel, usize idx, usize dim = 0uz) noexcept -> void {
			this->pixel = pixel;
			this->idx = idx;
			this->dim = dim;
		}

		auto generate_1d() const noexcept -> f32 {
			return dist(rng);
		}

		auto generate_2d() const noexcept -> Vector<f32, 2> {
			return {generate_1d(), generate_1d()};
		}

		auto generate_pixel_2d() const noexcept -> Vector<f32, 2> {
			return generate_2d();
		}
	
	private:
		mutable std::mt19937 rng;
		mutable std::uniform_real_distribution<f32> dist;
		Vector<usize, 2> pixel;
		usize idx;
		usize dim;
	};
}
