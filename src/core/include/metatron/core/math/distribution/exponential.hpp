#pragma once
#include <cmath>

namespace mtt::math {
	struct Exponential_Distribution final {
		Exponential_Distribution(f32 a) noexcept: a(a) {}

		auto sample(f32 u) const noexcept -> f32 {
			return -std::log(1.f - u) / a;
		}

		auto pdf(f32 x) const noexcept -> f32 {
			return a * std::exp(-a * x);
		}

	private:
		f32 a;
	};
}
