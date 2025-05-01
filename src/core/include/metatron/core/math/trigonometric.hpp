#pragma once
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <cmath>

namespace metatron::math {
	auto inline sinc(f32 x) -> f32 {
		return guarded_div(std::sin(pi * x), pi * x);
	};
}
