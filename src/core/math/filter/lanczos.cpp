#include <metatron/core/math/filter/lanczos.hpp>

namespace metatron::math {
	auto sinc(f32 x) -> f32 {
		return std::sin(pi * x) / (pi * x);
	}

	auto lanczos(f32 x, f32 r, f32 tau) -> f32 {
		if (std::abs(x) >= r) {
			return 0.f;
		}
		return sinc(x) * sinc(x / tau);
	}

	Lanczos_Filter::Lanczos_Filter(Vector<f32, 2> const& radius, f32 tau): radius(radius), tau(tau) {
		auto matrix = math::Matrix<f32, 64, 64>{};
		for (auto i = 0uz; i < 64; i++) {
			auto x = std::lerp(-radius[0], radius[0], (f32(i) + 0.5f) / 64.f);
			for (auto j = 0uz; j < 64; j++) {
				auto y = std::lerp(-radius[1], radius[1], (f32(j) + 0.5f) / 64.f);
				matrix[i][j] = (*this)({x, y});
			}
		}
		distribution = Piecewise_Distribution<64, 64>{
			std::move(matrix),
			{-radius[0], -radius[1]},
			{radius[0], radius[1]}
		};
	}

	auto Lanczos_Filter::operator()(Vector<f32, 2> const& p) const -> f32 {
		auto vx = lanczos(p[0], radius[0], tau);
		auto vy = lanczos(p[1], radius[1], tau);
		return vx * vy;
	}

	auto Lanczos_Filter::sample(Vector<f32, 2> const& u) const -> std::optional<filter::Interaction> {
		auto p = distribution.sample(u);
		auto w = (*this)(p);
		auto pdf = distribution.pdf(p);
		return filter::Interaction{p, w, pdf};
	}
}
