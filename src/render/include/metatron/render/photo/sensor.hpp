#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <metatron/core/color/color-space.hpp>

namespace metatron::photo {
	struct Sensor final {
		Sensor(color::Color_Space const* color_space);

		auto operator()(spectra::Stochastic_Spectrum const& spectrum) -> math::Vector<f32, 3>;

	private:
		color::Color_Space const* color_space;
	};
}
