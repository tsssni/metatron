#pragma once
#include <metatron/core/math/filter/filter.hpp>
#include <metatron/core/math/gaussian.hpp>
#include <metatron/core/math/distribution/piecewise.hpp>

namespace mtt::math {
	struct Gaussian_Filter final {
		Gaussian_Filter(Vector<f32, 2> const& radius = {1.5f}, f32 sigma = 0.5f) noexcept:
		radius(radius) {
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

		auto operator()(Vector<f32, 2> const& p) const noexcept -> f32 {

			auto vx = gaussian(p[0], 0.f, sigma) - gaussian(radius[0], 0.f, sigma);
			auto vy = gaussian(p[1], 0.f, sigma) - gaussian(radius[1], 0.f, sigma);
			return vx * vy;
		}

		auto sample(Vector<f32, 2> const& u) const noexcept -> std::optional<filter::Interaction> {
			auto p = piecewise.sample(u);
			auto w = (*this)(p);
			auto pdf = piecewise.pdf(p);
			return filter::Interaction{p, w, pdf};
		}

	private:
		Piecewise_Distribution<64, 64> piecewise;
		Vector<f32, 2> radius;
		f32 sigma;
	};
}
