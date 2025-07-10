#pragma once
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/resource/color/color-space.hpp>

namespace mtt::photo {
	struct Sensor final {
		Sensor(color::Color_Space const* color_space) noexcept;

		auto operator()(spectra::Stochastic_Spectrum const& spectrum) const noexcept-> math::Vector<f32, 3>;

	private:
		color::Color_Space const* color_space;
	};
}
