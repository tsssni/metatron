#pragma once
#include <metatron/core/math/constant.hpp>
#include <cmath>

namespace metatron::math {
	auto inline guarded_div(f32 x, f32 y) -> f32 {
		return std::abs(y) < epsilon<f32> ? 0.f : x / y;
	};
}
