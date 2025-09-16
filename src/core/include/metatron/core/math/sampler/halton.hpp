#pragma once
#include <metatron/core/math/sampler/sampler.hpp>
#include <metatron/core/math/prime.hpp>

namespace mtt::math {
	// halton with owen scrambling: https://pbr-book.org/4ed/Sampling_and_Reconstruction/Halton_Sampler
	struct Halton_Sampler final {
		Halton_Sampler(
			usize seed = 0uz,
			Vector<usize, 2> const& scale_exponential = {7uz, 4uz}
		) noexcept;
		Halton_Sampler(Halton_Sampler const&) noexcept = default;

		auto start(math::Vector<usize, 2> const& pixel, usize idx, usize dim = 2uz) noexcept -> void;
		auto generate_1d() const noexcept -> f32;
		auto generate_2d() const noexcept -> math::Vector<f32, 2>;
		auto generate_pixel_2d() const noexcept -> math::Vector<f32, 2>;
	
	private:
		Vector<usize, 2> pixel;
		Vector<usize, 2> exponential;
		Vector<usize, 2> scale;
		Vector<usize, 2> scale_mulinv;
		usize seed;
		usize idx;
		usize stride;
		usize mutable halton_idx;
		usize mutable dim;
	};
}
