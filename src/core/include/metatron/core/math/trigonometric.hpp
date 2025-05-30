#pragma once
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <cmath>

namespace metatron::math {
	auto inline sinc(f32 x) -> f32 {
		return guarded_div(std::sin(pi * x), pi * x);
	};

	auto inline windowed_sinc(f32 x, f32 tau) -> f32 {
		return sinc(x) * sinc(x / tau);
	}
}
