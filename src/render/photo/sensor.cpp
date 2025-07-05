#include <metatron/render/photo/sensor.hpp>

namespace mtt::photo {
	Sensor::Sensor(color::Color_Space const* color_space): color_space(color_space) {}

	auto Sensor::operator()(spectra::Stochastic_Spectrum const& spectrum) -> math::Vector<f32, 3> {
		auto xyz = math::Vector<f32, 3>{
			spectrum(*spectra::Spectrum::CIE_X),
			spectrum(*spectra::Spectrum::CIE_Y),
			spectrum(*spectra::Spectrum::CIE_Z),
		};
		auto rgb = color_space->from_XYZ | xyz;
		return rgb;
	}
}
