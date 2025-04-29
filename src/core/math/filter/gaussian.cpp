#include <metatron/core/math/filter/gaussian.hpp>

namespace metatron::math {
	auto gaussian(f32 x, f32 sigma) -> f32 {
		return std::exp(-x * x / (2.f * sigma * sigma)) / (std::sqrt(2.f * pi) * sigma);
	}

	Gaussian_Filter::Gaussian_Filter(Vector<f32, 2> const& radius, f32 sigma): radius(radius), sigma(sigma) {
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

	auto Gaussian_Filter::operator()(Vector<f32, 2> const& p) const -> f32 {
		auto vx = gaussian(p[0], sigma) - gaussian(radius[0], sigma);
		auto vy = gaussian(p[1], sigma) - gaussian(radius[1], sigma);
		return vx * vy;
	}

	auto Gaussian_Filter::sample(Vector<f32, 2> const& u) const -> std::optional<filter::Interaction> {
		auto p = distribution.sample(u);
		auto w = (*this)(p);
		auto pdf = distribution.pdf(p);
		return filter::Interaction{p, w, pdf};
	}
}
