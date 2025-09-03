#pragma once
#include <metatron/core/math/arithmetic.hpp>

namespace mtt::math {
	template<typename T>
	requires std::floating_point<T>
	auto gaussian(T x, T mu, T sigma) noexcept -> T {
		return std::exp(-math::sqr(x - mu) / (T{2} * math::sqr(sigma))) / (math::sqrt(T{2} * pi) * sigma);
	}

	template<typename T>
	requires std::floating_point<T>
	auto erf() noexcept -> T {
		if constexpr (std::is_same_v<T, f32>) {

		} else {

		}
	}
}
