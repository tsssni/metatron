#pragma once
#include <metatron/core/math/arithmetic.hpp>

namespace metatron::math {
	struct Linear_Distribution final {
		Linear_Distribution(f32 a, f32 b)
		: a(a), b(b) {}

		auto sample(f32 u) const -> f32 {
			auto a2 = a * a;
			auto b2 = b * b;
			auto x =  u * (a + b) / (a + math::sqrt(math::lerp(a2, b2, u)));
			return x;
		}

		auto pdf(f32 x) const -> f32 {
			return 2.f * std::lerp(a, b, x) / (a + b);
		}

	private:
		f32 a;
		f32 b;
	};
}
