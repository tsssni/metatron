#pragma once
#include <metatron/core/spectra/spectrum.hpp>

namespace metatron::spectra {
	struct Constant_Spectrum final: Spectrum {
		f32 x;
		Constant_Spectrum(f32 x);
		auto operator()(f32 lambda) const -> f32;
	};
}
