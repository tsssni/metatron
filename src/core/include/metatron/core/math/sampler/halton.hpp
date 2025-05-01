#pragma once
#include <metatron/core/math/sampler/sampler.hpp>
#include <metatron/core/math/prime.hpp>
#include <metatron/core/stl/optional.hpp>

namespace metatron::math {
	auto inline radical_inverse(usize x, usize b) -> f32 {
		auto reversed = 0uz;
		auto limit = (~0uz - b) / b;
		auto inv_b = 1uz;

		while (x && reversed < limit) {
			auto next = x / b;
			auto digit = x - next * b;
			reversed = reversed * b + digit;
			inv_b *= b;
			x = next;
		}

		return std::min(reversed / f32(inv_b), 1.f - epsilon<f32>);
	};

	auto inline inverse_radical(usize reversed, usize b, usize n) -> usize {
		auto idx = 0uz;
		for (auto i = 0uz; i < n; i++) {
			auto digit = reversed % b;
			reversed /= b;
			idx = idx * b + digit;
		}
		return idx;
	}

	auto inline halton(usize idx, usize b_idx) -> f32 {
		return radical_inverse(idx, primes[b_idx]);
	}

	struct Halton_Sampler final: Sampler {
		Halton_Sampler(
			Vector<usize, 2> const& scale_exponential = {7uz, 4uz}
		);

		auto start(math::Vector<usize, 2> const& pixel, usize idx, usize dim = 2uz) -> void;
		auto generate_1d() const -> f32;
		auto generate_2d() const -> math::Vector<f32, 2>;
		auto generate_pixel_2d() const -> math::Vector<f32, 2>;
	
	private:
		Vector<usize, 2> pixel;
		Vector<usize, 2> exponential;
		Vector<usize, 2> scale;
		Vector<usize, 2> scale_mulinv;
		usize stride;
		usize idx;
		usize mutable halton_idx;
		usize mutable dim;
	};
}
