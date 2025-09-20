#pragma once
#include <metatron/core/math/filter/filter.hpp>
#include <metatron/core/math/distribution/piecewise.hpp>
#include <metatron/core/math/trigonometric.hpp>

namespace mtt::math {
	struct Lanczos_Filter final {
		Lanczos_Filter(Vector<f32, 2> const& radius = {0.5f}, f32 tau = 3.f) noexcept:
		radius(radius), tau(tau) {
			auto matrix = math::Matrix<f32, 64, 64>{};
			for (auto i = 0uz; i < 64; i++) {
				auto x = math::lerp(-radius[0], radius[0], (f32(i) + 0.5f) / 64.f);
				for (auto j = 0uz; j < 64; j++) {
					auto y = math::lerp(-radius[1], radius[1], (f32(j) + 0.5f) / 64.f);
					matrix[i][j] = (*this)({x, y});
				}
			}
			distribution = Piecewise_Distribution<64, 64>{
				std::move(matrix),
				{-radius[0], -radius[1]},
				{radius[0], radius[1]}
			};
		}

		auto operator()(Vector<f32, 2> const& p) const noexcept -> f32 {
			auto v = foreach([&](f32 x, usize i) -> f32 {
				return math::abs(x) > radius[i] ? 0.f : windowed_sinc(x, tau);
			}, p);
			return prod(v);
		}

		auto sample(Vector<f32, 2> const& u) const noexcept -> std::optional<filter::Interaction> {
			auto p = distribution.sample(u);
			auto w = (*this)(p);
			auto pdf = distribution.pdf(p);
			return filter::Interaction{p, w, pdf};
		}

	private:
		Piecewise_Distribution<64, 64> distribution;
		Vector<f32, 2> radius;
		f32 tau;
	};
}
