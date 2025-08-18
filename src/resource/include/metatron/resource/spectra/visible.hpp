#pragma once
#include <metatron/resource/spectra/spectrum.hpp>

namespace mtt::spectra {
	auto constexpr visible_range = usize(visible_lambda[1] - visible_lambda[0] + 1.f);

	struct Visible_Spectrum final {
		Visible_Spectrum(std::array<f32, visible_range>&& data) noexcept;
		auto operator()(f32 lambda) const noexcept -> f32;
		auto static from_spectrum(view<Spectrum> spec, bool normalize = false) noexcept -> Visible_Spectrum;

	private:
		std::array<f32, visible_range> data;
	};
}
