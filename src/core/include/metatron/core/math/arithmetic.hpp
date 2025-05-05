#pragma once
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/vector.hpp>
#include <cmath>

namespace metatron::math {
	auto inline guarded_div(f32 x, f32 y) -> f32 {
		return std::abs(y) < epsilon<f32> ? 0.f : x / y;
	}

	template<usize n>
	auto inline guarded_div(Vector<f32, n> const& x, f32 y) -> Vector<f32, n> {
		return std::abs(y) < epsilon<f32> ? Vector<f32, n>{0.f} : x / y;
	}

	template<usize n>
	auto inline guarded_div(Vector<f32, n> const& x, Vector<f32, n> const& y) -> Vector<f32, n> {
		return foreach(x, [&y](f32 x, usize idx) -> f32 {
			return guarded_div(x, y[idx]);
		});
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
}
