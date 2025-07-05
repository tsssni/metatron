#pragma once
#include <vector>

namespace mtt::math {
	struct Discrete_Distribution final {
		Discrete_Distribution(std::vector<f32> const& weights): weights(weights) {
			for (auto w: weights) {
				sum_w += w;
			}
		}

		auto sample(f32 u) const -> usize {
			u *= sum_w;
			auto sum = 0.f;
			auto i = 0uz;
			for (; i < weights.size(); i++) {
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
		std::vector<f32> weights;
	};
}
