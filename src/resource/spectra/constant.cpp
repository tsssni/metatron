#include <metatron/resource/spectra/constant.hpp>

namespace mtt::spectra {
	Constant_Spectrum::Constant_Spectrum(f32 x): x(x) {}

	auto Constant_Spectrum::operator()(f32 lambda) const -> f32 {
		return x;
	}
}
