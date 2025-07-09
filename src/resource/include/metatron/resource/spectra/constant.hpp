#pragma once

namespace mtt::spectra {
	struct Constant_Spectrum final {
		f32 x;
		Constant_Spectrum(f32 x);
		auto operator()(f32 lambda) const -> f32;
	};
}
