#include <metatron/core/spectra/rgb.hpp>

namespace metatron::spectra {
	Rgb_Spectrum::Rgb_Spectrum(math::Vector<f32, 3> const& rgb) :rgb(rgb) {}

	auto Rgb_Spectrum::operator()(f32 lambda) const -> f32 {
		// TODO: just test rgb
		if (lambda >= 380.f && lambda < 380.f + 400.f / 3.f) return rgb[0];
		else if (lambda >= 380.f + 400.f / 3.f && lambda < 380.f + 400.f * 2.f / 3.f) return rgb[1];
		else return rgb[2];
	}

	auto Rgb_Spectrum::operator()(Spectrum const& spectrum) const -> f32 {
		auto f = 0.f;
		for (auto i = 0; i < 3; i++) {
			f += rgb[i] * spectrum(380.f + i * 400.f / 3.f);
		}
		return f;
	}

	auto Rgb_Spectrum::operator*(f32 s) -> Spectrum& {
		rgb *= s;
		return *this;
	}

	auto Rgb_Spectrum::operator*(Spectrum const& spectrum) const -> Spectrum const& {
		return *this;
	}
}
