#pragma once
#include <cmath>

namespace metatron::math {
	struct Linear_Distribution final {
		Linear_Distribution(f32 a, f32 b, f32 l = 0.f, f32 r = 1.f)
		: a(a), b(b), l(l), r(r) {}

		auto sample(f32 u) const -> f32 {
			auto b_norm = a + (b - a) / (r - l);
			auto a2 = a * a;
			auto b2 = b * b;
			auto x =  u * (a + b_norm) / (a + std::sqrt(std::lerp(a2, b2, u)));
			return l + x * (r - l);
		}

		auto pdf(f32 x) const -> f32 {
			auto y = (x - l) / (r - l);
			auto b_norm = a + (b - a) / (r - l);
			return 2.f * std::lerp(a, b_norm, x) / (a + b_norm);
		}

	private:
		f32 a;
		f32 b;
		f32 l;
		f32 r;
	};
}
