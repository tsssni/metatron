#pragma once
#include <metatron/core/math/constant.hpp>
#include <cmath>

namespace metatron::math {
	template<typename T>
	requires std::floating_point<T>
	auto inline constexpr guarded_div(T x, T y) -> T {
		return std::abs(y) < epsilon<T> ? 0.0 : x / y;
	}

	auto inline constexpr pow(usize x, usize n) -> usize {
		auto y = 1uz;
		while (n) {
			if (n & 1) {
				y = y * x;
			}
			x = x * x;
			n >>= 1;
		}
		return y;
	}

	template<typename T>
	requires std::floating_point<T>
	auto constexpr sqrt(T x) -> T {
		return std::sqrt(std::max(0.f, x));
	}

	template<typename T>
	requires std::floating_point<T>
	auto constexpr lerp(T x, T y, T alpha) -> T {
		return (T{1.0} - alpha) * x + alpha * y;
	}
}
