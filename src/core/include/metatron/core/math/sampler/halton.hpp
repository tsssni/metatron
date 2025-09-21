#pragma once
#include <metatron/core/math/prime.hpp>
#include <metatron/core/math/low-discrepancy.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/number-theory.hpp>

namespace mtt::math {
	// halton with owen scrambling: https://pbr-book.org/4ed/Sampling_and_Reconstruction/Halton_Sampler
	struct Halton_Sampler final {
		Halton_Sampler(
			usize seed,
			Vector<i32, 2> const& scale_exponential = {7, 4}
		) noexcept:
		exponential(scale_exponential),
		scale({1 << scale_exponential[0], math::pow(3, scale_exponential[1])}),
		seed(seed) {
			stride = scale[0] * scale[1];
			scale_mulinv = scale * Vector<usize, 2>{
				multiplicative_inverse(scale[0], scale[1]),
				multiplicative_inverse(scale[1], scale[0])
			};
		}
		Halton_Sampler(Halton_Sampler const&) noexcept = default;

		auto start(Vector<usize, 2> const& pixel, usize idx, usize dim = 2) noexcept -> void {
			this->pixel = pixel;
			this->idx = idx;
			this->dim = std::clamp(dim, 2uz, primes.size() - 1uz);

			// high num_exponetial bits of radical_inverse(halton_index) equals pixel % (base ^ num_exoinential),
			// so low num_exponetial bits of halton_index equals radical_inverse(pixel % (base ^ num_exoinetial))
			auto halton_low_digits = foreach([&](usize x, usize i) -> usize {
				return inverse_radical(x, primes[i], exponential[i]);
			}, math::mod(Vector<i32, 2>{pixel}, scale));

			// halton_index≡halton_low_digits[i](mod base^num_exponential[i])
			// use precomputed multiplicative inverse of scale to evaluate CRT
			// halton_index = chinese_remainder_theorem(halton_low_digits, scale);
			halton_idx = sum(halton_low_digits * scale_mulinv) % stride;

			// each sample has a LCM stride as we use primes as bases
			halton_idx += idx * stride;
		}

		auto generate_1d() noexcept -> f32 {
			if (dim >= primes.size()) {
				dim = 2uz;
			}
			auto scrambled = owen_scrambled_radical_inverse(
				halton_idx, primes[dim], mix_bits(seed ^ dim)
			);
			dim++;
			return scrambled;
		}

		auto generate_2d() noexcept -> Vector<f32, 2> {
			return {generate_1d(), generate_1d()};
		}

		auto generate_pixel_2d() noexcept -> Vector<f32, 2> {
			// remove integer part by dividing scale
			return {
				radical_inverse(halton_idx >> exponential[0], primes[0]),
				radical_inverse(halton_idx / scale[1], primes[1])
			};
		}
	
	private:
		Vector<i32, 2> pixel;
		Vector<i32, 2> exponential;
		Vector<i32, 2> scale;
		Vector<i32, 2> scale_mulinv;
		i32 idx;
		i32 stride;
		usize seed;
		usize dim;
		usize halton_idx;
	};
}
