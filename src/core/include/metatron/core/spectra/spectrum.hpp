#pragma once

namespace metatron::spectra {
	struct Spectrum {
		virtual ~Spectrum() {}
		auto virtual operator()(f32 lambda) const -> f32 = 0;
	};
}
