#pragma once
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/vector.hpp>
#include <cmath>

namespace metatron::math {
	auto inline guarded_div(f32 x, f32 y) -> f32 {
		return std::abs(y) < epsilon<f32> ? 0.f : x / y;
	};

	template<usize n>
	auto inline guarded_div(math::Vector<f32, n> x, f32 y) -> math::Vector<f32, n> {
		return std::abs(y) < epsilon<f32> ? math::Vector<f32, n>{0.f} : x / y;
	};
}
