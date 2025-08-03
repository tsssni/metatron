#include <metatron/resource/photo/sensor.hpp>

namespace mtt::photo {
	Sensor::Sensor(view<color::Color_Space> color_space) noexcept: color_space(color_space) {}

	auto Sensor::operator()(spectra::Stochastic_Spectrum const& spectrum) const noexcept -> math::Vector<f32, 3> {
		auto xyz = math::Vector<f32, 3>{
			spectrum(spectra::Spectrum::spectra["CIE-X"]),
			spectrum(spectra::Spectrum::spectra["CIE-Y"]),
			spectrum(spectra::Spectrum::spectra["CIE-Z"]),
		};
		auto rgb = color_space->from_XYZ | xyz;
		return rgb;
	}
}
