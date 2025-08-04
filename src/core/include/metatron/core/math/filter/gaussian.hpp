#pragma once
#include <metatron/core/math/filter/filter.hpp>
#include <metatron/core/math/distribution/piecewise.hpp>
#include <metatron/core/math/distribution/gaussian.hpp>

namespace mtt::math {
	struct Gaussian_Filter final {
		Gaussian_Filter(Vector<f32, 2> const& radius = {1.5f}, f32 sigma = 0.5f) noexcept;

		auto operator()(Vector<f32, 2> const& p) const noexcept -> f32;
		auto sample(Vector<f32, 2> const& u) const noexcept -> std::optional<filter::Interaction>;

	private:
		Piecewise_Distribution<64, 64> piecewise;
		Gaussian_Distribution gaussian;
		Vector<f32, 2> radius;
		f32 sigma;
	};
}
