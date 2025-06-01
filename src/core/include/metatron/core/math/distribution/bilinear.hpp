#pragma once
#include <metatron/core/math/distribution/linear.hpp>
#include <metatron/core/math/vector.hpp>

namespace metatron::math {
	struct Bilinear_Distribution final {
		Bilinear_Distribution(f32 a, f32 b, f32 c, f32 d)
		: a(a), b(b), c(c), d(d) {}

		auto sample(math::Vector<f32, 2> u) const -> math::Vector<f32, 2> {
			auto x = Linear_Distribution{a + b, c + d}.sample(u[0]);
			auto y = Linear_Distribution{std::lerp(a, c, x), std::lerp(b, d, x)}.sample(u[1]);
			return {x, y};
		}

		auto pdf(math::Vector<f32, 2> x) const -> f32 {
			return 1.f
			* Linear_Distribution{a + b, c + d}.pdf(x[0])
			* Linear_Distribution{std::lerp(a, c, x[0]), std::lerp(b, d, x[0])}.pdf(x[1]);
		}

	private:
		f32 a;
		f32 b;
		f32 c;
		f32 d;
	};
}
