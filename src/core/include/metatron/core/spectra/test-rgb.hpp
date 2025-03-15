#pragma once
#include <metatron/core/spectra/spectrum.hpp>
#include <metatron/core/math/vector.hpp>

namespace metatron::spectra {
	struct Test_Rgb_Spectrum final: Spectrum {
		Test_Rgb_Spectrum(f32 min_lambda, f32 max_lambda);
		auto operator()(f32 lambda) const -> f32;

	private:
		f32 min_lambda;
		f32 max_lambda;
	};
}
