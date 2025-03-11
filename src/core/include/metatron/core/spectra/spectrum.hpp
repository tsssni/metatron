#pragma once

namespace metatron::spectra {
	struct Spectrum {
		virtual ~Spectrum() {}
		auto virtual operator()(f32 lambda) const -> f32 = 0;
		auto virtual operator()(Spectrum const& spectrum) const -> f32 = 0;
		auto virtual operator*(f32 s) -> Spectrum& = 0;
		auto virtual operator*(Spectrum const& spectrum) const -> Spectrum const& = 0;
	};
}
