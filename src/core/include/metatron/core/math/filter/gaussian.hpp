#pragma once
#include <metatron/core/math/filter/filter.hpp>
#include <metatron/core/math/distribution/piecewise.hpp>
#include <metatron/core/math/distribution/gaussian.hpp>

namespace metatron::math {
	struct Gaussian_Filter final: Filter {
		Gaussian_Filter(Vector<f32, 2> const& radius = {1.5f}, f32 sigma = 0.5f);

		auto operator()(Vector<f32, 2> const& p) const -> f32;
		auto sample(Vector<f32, 2> const& u) const -> std::optional<filter::Interaction>;

	private:
		Piecewise_Distribution<64, 64> piecewise;
		Gaussian_Distribution gaussian;
		Vector<f32, 2> radius;
		f32 sigma;
	};
}
