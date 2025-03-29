#pragma once
#include <metatron/core/spectra/spectrum.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/polynomial.hpp>

namespace metatron::spectra {
	struct Rgb_Spectrum final: Spectrum {
		Rgb_Spectrum(
			math::Vector<f32, 3> const& c,
			f32 s = 1.f,
			Spectrum const* illuminant = nullptr
		);
		auto operator()(f32 lambda) const -> f32;

	private:
		math::Polynomial<3> polynomial;
		f32 s;
		Spectrum const* illuminant;
	};
}
