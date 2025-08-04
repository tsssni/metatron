#include <metatron/core/math/filter/gaussian.hpp>

namespace mtt::math {
	Gaussian_Filter::Gaussian_Filter(
		Vector<f32, 2> const& radius,
		f32 sigma
	) noexcept:
	radius(radius),
	gaussian(sigma) {
		auto matrix = math::Matrix<f32, 64, 64>{};
		for (auto i = 0uz; i < 64; i++) {
			auto x = math::lerp(-radius[0], radius[0], (f32(i) + 0.5f) / 64.f);
			for (auto j = 0uz; j < 64; j++) {
				auto y = math::lerp(-radius[1], radius[1], (f32(j) + 0.5f) / 64.f);
				matrix[i][j] = (*this)({x, y});
			}
		}
		piecewise = Piecewise_Distribution<64, 64>{
			std::move(matrix),
			{-radius[0], -radius[1]},
			{radius[0], radius[1]}
		};
	}

	auto Gaussian_Filter::operator()(Vector<f32, 2> const& p) const noexcept -> f32 {
		auto vx = gaussian.pdf(p[0]) - gaussian.pdf(radius[0]);
		auto vy = gaussian.pdf(p[1]) - gaussian.pdf(radius[1]);
		return vx * vy;
	}

	auto Gaussian_Filter::sample(Vector<f32, 2> const& u) const noexcept -> std::optional<filter::Interaction> {
		auto p = piecewise.sample(u);
		auto w = (*this)(p);
		auto pdf = piecewise.pdf(p);
		return filter::Interaction{p, w, pdf};
	}
}
