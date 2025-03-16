#pragma once
#include <array>

namespace metatron::math {
	template<usize n>
	struct Discrete_Distribution final {
		Discrete_Distribution(std::array<f32, n> const& weights): weights(weights) {
			for (auto w: weights) {
				sum_w += w;
			}
		}

		auto sample(f32 u) -> usize {
			u *= sum_w;
			auto sum = 0.f;
			auto i = 0uz;
			for (; i < n; i++) {
				if (sum + weights[i] < u) {
					sum += weights[i];
				} else {
					break;
				}
			}
			return i;
		}

	private:
		f32 sum_w{0.f};
		std::array<f32, n> weights;
	};
}
