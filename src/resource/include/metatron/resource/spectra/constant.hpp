#pragma once

namespace mtt::spectra {
	struct Constant_Spectrum final {
		f32 x;
		Constant_Spectrum(f32 x) noexcept;
		auto operator()(f32 lambda) const noexcept -> f32;
	};
}
