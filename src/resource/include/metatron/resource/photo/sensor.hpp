#pragma once
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/resource/color/color-space.hpp>

namespace mtt::photo {
	struct Sensor final {
		Sensor(view<color::Color_Space> color_space) noexcept;

		auto operator()(spectra::Stochastic_Spectrum const& spectrum) const noexcept-> math::Vector<f32, 3>;

	private:
		view<color::Color_Space> color_space;
		view<spectra::Spectrum> r;
		view<spectra::Spectrum> g;
		view<spectra::Spectrum> b;
	};
}
