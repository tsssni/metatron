#pragma once
#include <cmath>

namespace metatron::math {
	struct Exponential_Distribution final {
		Exponential_Distribution(f32 a): a(a) {}

		auto operator()(f32 x) -> f32 {
			return a * std::exp(-a * x);
		}

		auto sample(f32 u) -> f32 {
			return -std::log(1.f - u) / a;
		}

	private:
		f32 a;
	};
}
