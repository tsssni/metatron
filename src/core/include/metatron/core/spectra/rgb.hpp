#pragma once
#include <metatron/core/spectra/spectrum.hpp>
#include <metatron/core/math/vector.hpp>

namespace metatron::spectra {
	struct Rgb_Spectrum final: Spectrum {
		mutable math::Vector<f32, 3> rgb;

		Rgb_Spectrum(math::Vector<f32, 3> const& rgb);

		auto operator()(f32 lambda) const -> f32;
		auto operator()(Spectrum const& spectrum) const -> f32;
		auto operator*(Spectrum const& spectrum) const -> Spectrum const&;
	};
}
