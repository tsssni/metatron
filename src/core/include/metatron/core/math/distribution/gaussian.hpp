#pragma once
#include <metatron/core/math/gaussian.hpp>

namespace mtt::math {
	struct Gaussian_Distribution final {
		Gaussian_Distribution(f32 mu, f32 sigma) noexcept: mu(mu), sigma(sigma) {}

		auto pdf(f32 x) const noexcept -> f32 {
			return gaussian(x, mu, sigma);
		}

	private:
		f32 mu;
		f32 sigma;
	};
}
