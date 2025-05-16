#pragma once
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <cmath>

namespace metatron::math {
	struct Gaussian_Distribution final {
		Gaussian_Distribution(f32 sigma): sigma(sigma) {}

		auto pdf(f32 x) const -> f32 {
			return std::exp(-x * x / (2.f * sigma * sigma)) / (math::sqrt(2.f * pi) * sigma);
		}

	private:
		f32 sigma;
	};
}
