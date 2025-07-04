#pragma once
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/prime.hpp>
#include <metatron/core/math/hash.hpp>

namespace metatron::math {
	auto inline constexpr radical_inverse(usize x, usize b) -> f32 {
		auto reversed = 0uz;
		auto limit = (~0uz - b) / b;
		auto inv_b = 1uz;

		// stop when next reversed will overflow
		while (x && reversed < limit) {
			auto next = x / b;
			auto digit = x - next * b;
			reversed = reversed * b + digit;
			inv_b *= b;
			x = next;
		}

		return std::min(reversed / f32(inv_b), 1.f - epsilon<f32>);
	};

	auto inline constexpr inverse_radical(usize reversed, usize b, usize n) -> usize {
		auto idx = 0uz;
		for (auto i = 0uz; i < n; i++) {
			auto digit = reversed % b;
			reversed /= b;
			idx = idx * b + digit;
		}
		return idx;
	}

	auto inline constexpr owen_scrambled_radical_inverse(usize x, usize b, usize hash) -> f32 {
		auto reversed = 0uz;
		auto limit = (~0uz - b) / b;
		auto inv_b = 1uz;

		// stop when precision of highest number in current bit not enough
		while (1.f - f32(b - 1) / inv_b < 1.f && reversed < limit) {
			auto next = x / b;
			auto digit = x - next * b;
			auto digit_hash = mix_bits(hash ^ reversed);
			reversed = reversed * b + biject_permute(digit, b, digit_hash);
			inv_b *= b;
			x = next;
		}

		return std::min(reversed / f32(inv_b), 1.f - epsilon<f32>);
	}
}
