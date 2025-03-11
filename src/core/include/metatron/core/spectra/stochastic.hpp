#pragma once
#include <metatron/core/spectra/spectrum.hpp>
#include <vector>

namespace metatron::spectra {
	struct Stochastic_Spectrum final: Spectrum {
		mutable std::vector<f32> lambda;
		mutable std::vector<f32> pdf;
		mutable std::vector<f32> value;

		Stochastic_Spectrum(usize n, f32 u);

		auto operator()(f32 lambda) const -> f32;
		auto operator()(Spectrum const& spectrum) const -> f32;
		auto operator*(f32 s) -> Spectrum&;
		auto operator*(Spectrum const& spectrum) const -> Spectrum const&;
	};
}
