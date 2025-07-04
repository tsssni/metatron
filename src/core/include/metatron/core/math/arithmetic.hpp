#pragma once
#include <metatron/core/math/constant.hpp>
#include <cmath>

namespace metatron::math {
	template<typename T>
	auto inline constexpr abs(T x) -> T {
		return std::abs(x);
	}

	template<typename T>
	requires std::floating_point<T>
	auto inline constexpr guarded_div(T x, T y) -> T {
		return abs(y) < epsilon<T> ? 0.0 : x / y;
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
	requires requires(T x) { {x * x} -> std::convertible_to<T>; }
	auto constexpr sqr(T x) -> T {
		return x * x;
	}

	template<typename T>
	requires std::floating_point<T>
	auto constexpr lerp(T x, T y, T alpha) -> T {
		return (T{1.0} - alpha) * x + alpha * y;
	}
}
