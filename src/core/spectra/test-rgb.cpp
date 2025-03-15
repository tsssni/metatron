#include <metatron/core/spectra/test-rgb.hpp>

namespace metatron::spectra {
	Test_Rgb_Spectrum::Test_Rgb_Spectrum(f32 min_lambda, f32 max_lambda) 
		: min_lambda(min_lambda), max_lambda(max_lambda) {}

	auto Test_Rgb_Spectrum::operator()(f32 lambda) const -> f32 {
		return f32(lambda >= min_lambda && lambda < max_lambda);
	}
}
