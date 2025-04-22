#pragma once
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/vector.hpp>
#include <cmath>

namespace metatron::math {
	auto inline guarded_div(f32 x, f32 y) -> f32 {
		return std::abs(y) < epsilon<f32> ? 0.f : x / y;
	};

	template<usize n>
	auto inline guarded_div(Vector<f32, n> const& x, f32 y) -> Vector<f32, n> {
		return std::abs(y) < epsilon<f32> ? Vector<f32, n>{0.f} : x / y;
	};

	template<usize n>
	auto inline guarded_div(Vector<f32, n> const& x, Vector<f32, n> const& y) -> Vector<f32, n> {
		auto z = Vector<f32, n>{}; 
		for (auto i = 0uz; i < n; i++) {
			z[i] = guarded_div(x, y);
		}
		return z;
	};
}
