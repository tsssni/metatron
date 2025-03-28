#include <metatron/core/spectra/rgb.hpp>

namespace metatron::spectra {
	Rgb_Spectrum::Rgb_Spectrum(
		math::Vector<f32, 3> const& c,
		Spectrum const* illuminant
	): polynomial({c[0], c[1], c[2]}), illuminant(illuminant) {}

	auto Rgb_Spectrum::operator()(f32 lambda) const -> f32 {
		auto sigmoid = [](f32 x) -> f32 {
			return 0.5f + x / (2.f * std::sqrt(1.f + x * x));
		};
		return sigmoid(polynomial(lambda)) * (illuminant ? (*illuminant)(lambda) : 1.f);
	}
}
