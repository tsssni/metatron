#pragma once
#include <metatron/core/math/constant.hpp>
#include <cmath>

namespace metatron::math {
	template<typename T>
	requires std::floating_point<T>
	auto inline guarded_div(T x, T y) -> T {
		return std::abs(y) < epsilon<T> ? 0.0 : x / y;
	}

	auto inline pow(usize x, usize n) -> usize {
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
	auto sqrt(T x) -> T {
		return std::sqrt(std::max(0.f, x));
	}

	template<typename T>
	requires std::floating_point<T>
	auto lerp(T x, T y, T alpha) -> T {
		return (T{1.0} - alpha) * x + alpha * y;
	}
}
