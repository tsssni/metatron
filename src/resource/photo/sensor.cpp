#include <metatron/resource/photo/sensor.hpp>

namespace mtt::photo {
	Sensor::Sensor(view<color::Color_Space> color_space) noexcept:
	color_space(color_space),
	r(spectra::Spectrum::spectra["CIE-X"]),
	g(spectra::Spectrum::spectra["CIE-Y"]),
	b(spectra::Spectrum::spectra["CIE-Z"]),
	// scale is needed as we use white point with Y = 1
	s(spectra::Spectrum::spectra["CIE-Y"] | color_space->illuminant) {}

	auto Sensor::operator()(spectra::Stochastic_Spectrum const& spectrum) const noexcept -> math::Vector<f32, 3> {
		auto xyz = math::Vector<f32, 3>{
			spectrum(r),
			spectrum(g),
			spectrum(b),
		} / s;
		auto rgb = color_space->from_XYZ | xyz;
		return rgb;
	}
}
