#pragma once
#include <metatron/resource/spectra/spectrum.hpp>

namespace metatron::spectra {
	auto constexpr visible_range = usize(visible_lambda[1] - visible_lambda[0] + 1.f);

	struct Visible_Spectrum final: Spectrum {
		Visible_Spectrum(std::array<f32, visible_range>&& data);
		auto operator()(f32 lambda) const -> f32;

	private:
		std::array<f32, visible_range> data;
	};
}
