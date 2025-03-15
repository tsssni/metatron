#include <metatron/core/spectra/rgb.hpp>

namespace metatron::spectra {
	Rgb_Spectrum::Rgb_Spectrum(math::Vector<f32, 3> const& rgb) :rgb(rgb) {}

	auto Rgb_Spectrum::operator()(f32 lambda) const -> f32 {
		// TODO: just test rgb
		if (lambda >= 300.f && lambda < 450.f) return rgb[0];
		else if (lambda >= 450.f && lambda < 600.f) return rgb[1];
		else return rgb[2];
	}
}
